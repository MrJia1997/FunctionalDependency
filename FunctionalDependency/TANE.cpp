#include "TANE.h"
#include <fstream>
#include <algorithm>
using namespace std;
static string table[100000][16];
#define l L[level]
#define eps 0

TANE::TANE(int row, int column)
{
	this->column = column;
	this->row = row;
	max_level = 100;
	level = 1;
	shift = new int[column + 1];
	for (int i = 0; i <= column; i++) {
		shift[i] = 1 << i;
	}
	R = shift[column] - 1;
	cplus = new int[shift[column]];
	cplus[0] = R;
	result = new int[shift[column]];
	T = new int[shift[column]];
	E = new double*[shift[column]];
	for (int i = 0; i < column; i++) {
		E[i] = new double[shift[column]];
	}
	S = new vector<int>[column];
	init_pi = new map<string, vector<int>>[column];
	pi = new vector<vector<int>>[shift[column]];
	L = new vector<int>[max_level];
	L[0].clear();
	for (int i = 0; i < column; i++) {
		L[1].push_back(shift[i]);
	}
}

TANE::~TANE()
{
	delete L;
}

int CountOne(int v)//计算1的个数
{
	int num = 0;
	while (v)
	{
		v &= (v - 1);
		num++;
	}
	return num;
}

bool TANE::Superkey(int X) {

	return GetEValueOfSingle(X) <= eps;
}

double TANE::GetExactEValue(int X, int A)
{
	int e = 0, size;
	size = pi[X | A].size();
	for (int i = 0; i < size; i++) {
		T[pi[X | A][i][0]] = pi[X | A][i].size();
	}
	size = pi[X].size();
	for (int i = 0; i < size; i++) {
		int m = 1;
		int csize = pi[X][i].size();
		for (int j = 0; j < csize; j++) {
			m = m >= T[pi[X][i][j]] ? m : T[pi[X][i][j]];
		}
		e = e + csize - m;
	}
	size = pi[X | A].size();
	for (int i = 0; i < size; i++) {
		T[pi[X | A][i][0]] = 0;
	}
	return e / row;
}

double TANE::GetEValueOfSingle(int X)
{
	int spi = 0, sspi = 0;
	spi = pi[X].size();
	for (int i = 0; i < spi; i++) {
		sspi += pi[X][i].size();
	}
	return (sspi - spi) / row;
}

void TANE::GetFunctionDependence()
{
	while (!L[level].empty()) {
		sort(L[level].begin(), L[level].end());
		ComputeDependencies(level);
		Prune(level);
		GenerateNextLevel(level);
		level++;
	}
}

void TANE::ComputeDependencies(int size)
{
	int len = l.size();
	double exact_value = -1;
	for (int i = 0; i < len; i++) {
		int X = l[i];
		for (int j = 0; j < column; j++) {
			int A = shift[j];
			if (A & X) {
				cplus[X] = cplus[X] & cplus[X - A];
			}
		}
	}
	for (int i = 0; i < len; i++) {
		int X = l[i];
		for (int j = 0; j < column; j++) {
			int A = shift[j];
			if (A &(X & cplus[X])) {
				if (GetEValueOfSingle(X) - GetEValueOfSingle(X | A) > eps)continue;
				if( GetEValueOfSingle(X) <= eps ||
					(exact_value = GetExactEValue(X-A, A)) <= eps) {//X\{A}->A is valid
					//output X-A->A
					result[X - A] = result[X - A] | A;
					cplus[X] = cplus[X] - A;
					if (exact_value < 0)exact_value = GetExactEValue(X - A, A);
					if (exact_value == 0) {
						for (int k = 0; k < column; k++) {
							int B = shift[k];
							if (B & (R - X)) {
								cplus[X] = cplus[X] - B;
							}
						}
					}
				}
			}
		}
	}
}

void TANE::Prune(int level)
{
	int len = l.size();
	for (vector <int>::iterator i = l.begin(); i != l.end(); ) {
		int X = *i;
		if (!cplus[X]) {
			vector <int>::iterator j = i + 1;
			l.erase(i);
			i = j;
		}
		if (Superkey(X)) {
			for (int j = 0; j < column; j++) {
				int A = shift[j];
				int state = R;
				if (A & (cplus[X] - X)) {
					for (int k = 0; k < column; k++) {
						int B = shift[k];
						if (B &X) {
							state = state & cplus[(X | A) - B];
						}
					}
					if (A & state) {
						//output X->A
					}
				}
			}
			vector <int>::iterator j = i + 1;
			l.erase(i);
			i = j;
		}
	}
}

void TANE::GenerateNextLevel(int level)
{
	int len, state;
	int last = 0;
	int next = level + 1;
	//Prefix_Block
	tmp.push_back(l[last]);
	for (int i = 1; i < len; i++) {//l已经排好序
		int same = l[i] & l[last];
		if (CountOne(same) == level - 1 && l[last] - same > same) { //有level-1个相同元素，且不同的元素在最后一位
			tmp.push_back(l[i]);
		}
		else {
			len = tmp.size();
			for (int i = 0; i < len; i++) { //Y
				for (int j = i + 1; j < len; j++) {//Z
					state = tmp[i] | tmp[j];
					StrippedProduct(tmp[i], tmp[j]);
					for (int k = 0; k < column; k++) { //A
						int A = shift[k];
						if ((A & state) && find(l.begin(), l.end(), state-A) != l.end()) {
							L[next].push_back(state);
						}
					}
				}
			}
			tmp.clear();
			last = i;
			tmp.push_back(l[last]);
		}
	}
	len = tmp.size();
	for (int i = 0; i < len; i++) { //Y
		for (int j = i + 1; j < len; j++) {//Z
			state = tmp[i] | tmp[j];//X
			for (int k = 0; k < column; k++) { //A
				int A = shift[k];
				if ((A & state) && find(l.begin(), l.end(), state - A) != l.end()) {
					L[next].push_back(state);
				}
			}
		}
	}
}

void TANE::StrippedInit()
{
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			init_pi[j][table[i][j]].push_back(i);
		}
	}
	map<string, vector<int>>::iterator iter;
	for (int i = 0; i < column; i++) {
		for (iter = init_pi[i].begin(); iter != init_pi[i].end(); iter++) {
			pi[shift[i]].push_back(iter->second);
		}
	}
	for (int i = 0; i < R; i++) {
		T[i] = 0;
	}
}

void TANE::StrippedProduct(int Y, int Z)
{
	int xset = Y | Z;
	pi[xset].clear();
	int ysize = pi[Y].size(), zsize = pi[Z].size;
	for (int i = 0; i < ysize; i++) {
		int size = pi[Y][i].size();
		for (int j = 0; j < size; j++) {
			int t = pi[Z][i][j];
			T[t] = i;
		}
		S[i].clear();
	}
	for (int i = 0; i < zsize; i++) {
		int size = pi[Z][i].size();
		for (int j = 0; j < size; j++) {
			int t = pi[Z][i][j];
			if (T[t] != 0) {
				S[T[t]].push_back(t);
			}
		}
		for (int j = 0; j < size; j++) {
			int t = pi[Z][i][j];
			if (S[T[t]].size() >= 2) {
				pi[xset].push_back(S[T[t]]);
				S[T[t]].clear();
			}
		}
	}
	for (int i = 0; i < ysize; i++) {
		int size = pi[Y][i].size();
		for (int j = 0; j < size; j++) {
			int t = pi[Y][i][j];
			T[t] = 0;
		}
	}
}

