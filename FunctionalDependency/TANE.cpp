#include "TANE.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include<cmath>
using namespace std;
extern vector<vector<string>> table;
#define l L[level]
#define eps 0 // 0~0.1 * row 的整数

TANE::TANE()
{
	column = table[0].size();
	row = table.size();
	max_level = 1000;
	shift = new int[column + 1];
	for (int i = 0; i <= column; i++) {
		shift[i] = 1 << i;
	}
	R = shift[column] - 1;
	cplus = new int[shift[column]]; //都是& 初始值应该为R
	vis = new int[shift[column]]{0}; 
	result_vis = new int[shift[column]]{ 0 };
	T = new int[shift[column] > row+1 ? shift[column]:row +1]{0};
	S = new vector<int>[row+1];
	result_right = new vector<int>[shift[column]];
	init_pi = new unordered_map<string, vector<int>>[column];
	pi = new vector<vector<int>>[shift[column]];
	L = new vector<int>[max_level];
}

TANE::~TANE()
{
}

int CountOne(int v)//判断v中有多少个1，用来比较前缀
{
	int num = 0;
	while (v)
	{
		v &= (v - 1);
		num++;
	}
	return num;
}

bool LexicoCmp(const int &a, const int &b) { //拆成字典序
	int x = a, y = b;
	while ((x&1)== (y&1))
	{
		x = x >> 1;
		y = y >> 1;
		//if (x == 0)return y > x; 注意a和b中1的个数相同
		//if (y == 0)return x > y;
	}
	return (x & 1) > (y & 1);
}
bool LexicoCmpOut(const int &a, const int &b) { //拆成字典序
	int x = a, y = b;
	while ((x & 1) == (y & 1))
	{
		x = x >> 1;
		y = y >> 1;
		if (x == 0)return y < x; 
		if (y == 0)return x < y;
	}
	return (x & 1) > (y & 1);
}

void TANE::OutputFD() {
	ofstream Outfile("result.txt");
	int lsize = result_left.size();
	sort(result_left.begin(), result_left.end(), LexicoCmpOut);
	for (int k = 0; k < lsize; k++) {
		int X = result_left[k];
		sort(result_right[X].begin(), result_right[X].end());
		int rsize = result_right[X].size();
		for (int i = 0; i < rsize; i++) {
			for (int j = 0; j < column; j++) {
				if (X & shift[j]) {
					Outfile << j+1 << " ";
				}
			}
			Outfile << "-> ";
			Outfile << result_right[X][i] << endl;
		}
	}
	Outfile.close();
}

bool TANE::Superkey(int X) {
	return pi[X].size() == 0;
}

int TANE::GetExactEValue(int X, int A)
{
	int e = 0, size;
	int XA = X | A;
	size = pi[XA].size();
	for (int i = 0; i < size; i++) {
		T[pi[XA][i][0]] = pi[XA][i].size();
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
	size = pi[XA].size();
	for (int i = 0; i < size; i++) {
		T[pi[XA][i][0]] = 0;
	}
	return e ;
}

int TANE::GetEValueOfSingle(int X)
{
	int spi = 0, sspi = 0;
	spi = pi[X].size();
	for (int i = 0; i < spi; i++) {
		sspi += pi[X][i].size();
	}
	return (sspi - spi) ; 
}

void TANE::GetFunctionDependence()
{
	level = 1;
	for (int i = 0; i < shift[column]; i++) {
		cplus[i] = R;
	}
	L[0].clear();
	for (int i = 0; i < column; i++) {
		L[1].push_back(shift[i]);
	}
	// << 0 << endl;
	StrippedInit();
	while (!L[level].empty()) {
		//cout << level << endl;
		//cout << L[level].size() << endl;
		ComputeDependencies(level);
		Prune(level);
		sort(L[level].begin(), L[level].end(), LexicoCmp);
		GenerateNextLevel(level);
		level++;
	}
}

void TANE::ComputeDependencies(int size)
{
	int len = l.size();
	for (int i = 0; i < len; i++) {
		int X = l[i];
		for (int j = 0; j < column; j++) {
			int A = shift[j];
			if (A & X) {
				cplus[X] = cplus[X] & cplus[X - A];
			}
		}
	}
	//if (level == 1)return;
	for (int i = 0; i < len; i++) {
		int X = l[i];
		for (int j = 0; j < column; j++) {
			int A = shift[j];
			if (A &(X & cplus[X])) {
				if (X == A)continue;
				int EX = GetEValueOfSingle(X);
				int EXSubA = GetEValueOfSingle(X - A);
				if (EXSubA - EX > eps)continue;
				if (EXSubA <= eps ||
					GetExactEValue(X-A, A) <= eps) {//X\{A}->A is valid
					//OutputFD(X - A, A);
					if (!result_vis[X - A]) {
						result_left.push_back(X - A);
						result_vis[X - A] = 1;
					}
					result_right[X - A].push_back(j+1);
					cplus[X] = cplus[X] - A;
					if (EXSubA == EX) {
						for (int k = 0; k < column; k++) {
							int B = shift[k];
							if ((B & cplus[X]) && (B & (R - X))) {
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
	for (vector <int>::iterator i = l.begin(); i != l.end();) {
		int X = *i;
		if (!cplus[X]) {
			i = l.erase(i);
		}
		else if (Superkey(X)) {
			for (int j = 0; j < column; j++) {
				int A = shift[j];
				if (A & (cplus[X] - (cplus[X]&X))) {
					int state = 0, k;
					for (k = 0; k < column; k++) {
						int B = shift[k];
						if (B & X) {
							state = cplus[(X | A) - B];
							break;
						}
					}
					for (; k < column; k++) {
						int B = shift[k];
						if (B & X) {
							state = state & cplus[(X | A) - B];
						}
					}
					if (A & state) {
						//output X->A
						//OutputFD(X, A);
						if (!result_vis[X]) {
							result_left.push_back(X);
							result_vis[X] = 1;
						}
						result_right[X].push_back(j+1);
					}
				}
			}
			i = l.erase(i);
		}
		else {
			i++;
		}
	}
}

void TANE::GenerateNextLevel(int level)
{
	int len, tlen, state;
	int last = 0;
	int next = level + 1;
	//Prefix_Block
	L[next].clear();
	len = l.size();
	if (!len) return;
	tmp.push_back(l[last]); //K
	for (int i = 1; i < len; i++) {
		int same = l[i] & l[last];
		if (CountOne(same) == level - 1 && l[last] - same > same) {
			tmp.push_back(l[i]);
		}
		else {
			tlen = tmp.size();
			for (int i = 0; i < tlen; i++) { //Y
				for (int j = i + 1; j < tlen; j++) {//Z
					state = tmp[i] | tmp[j];
					StrippedProduct(tmp[i], tmp[j]);
					int flag = 1;
					for (int k = 0; k < column; k++) { //A
						int A = shift[k];
						if (A & state) {
							if (find(l.begin(), l.end(), state - A) == l.end()) {
								flag = 0;
								break;
							}
						}
					}
					if (flag && !vis[state]) {
						L[next].push_back(state);
						vis[state] = 1;
					}
				}
			}
			tmp.clear();
			last = i;
			tmp.push_back(l[last]);
		}
	}
	tlen = tmp.size();
	for (int i = 0; i < tlen; i++) { //Y
		for (int j = i + 1; j < tlen; j++) {//Z
			state = tmp[i] | tmp[j];//X
			StrippedProduct(tmp[i], tmp[j]);
			int flag = 1;
			for (int k = 0; k < column; k++) { //A
				int A = shift[k];
				if (A & state) {
					if (find(l.begin(), l.end(), state - A) == l.end()) {
						flag = 0;
						break;
					}
				}
			}
			if (flag && !vis[state]) {
				L[next].push_back(state);
				vis[state] = 1;
			}
		}
	}
	tmp.clear();
}

void TANE::StrippedInit()
{
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			init_pi[j][table[i][j]].push_back(i+1);
		}
	}
	unordered_map<string, vector<int>>::iterator iter;
	for (int i = 0; i < column; i++) {
		for (iter = init_pi[i].begin(); iter != init_pi[i].end(); iter++) {
			if (iter->second.size() >= 2) {
				pi[shift[i]].push_back(iter->second);
			}
		}
	}
	for (int i = 0; i < R; i++) {
		T[i] = 0;
	}
}

void TANE::StrippedProduct(int Y, int Z)
{
	int X = Y | Z;
	pi[X].clear();
	int ysize = pi[Y].size(), zsize = pi[Z].size();
	for (int i = 0; i < ysize; i++) {
		int suci = i + 1;
		int size = pi[Y][i].size(); //ci
		for (int j = 0; j < size; j++) {
			int t = pi[Y][i][j];
			T[t] = suci;
		}
		S[suci].clear();
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
				pi[X].push_back(S[T[t]]);
			}
			S[T[t]].clear();
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

