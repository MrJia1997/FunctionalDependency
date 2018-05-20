#include "TANE.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <thread>
#include <cmath>
#include <ctime>
using namespace std;
extern vector<vector<string>> table;
#define l L[level]
#define eps 0 // 0~0.1 * row 的整数
vector<int> *element; //element of bitset i 
TANE::TANE() 
{
	column = table[0].size();
	row = table.size();
	maxlevel = 10;
	powerTow = new int[column + 1]{ 0 };
	for (int i = 0; i <= column; i++) {
		powerTow[i] = 1 << i;
	}
	R = powerTow[column] - 1;
	cplus = new int[powerTow[column]]{ 0 };
	levelIn = new int[powerTow[column]]{ 0 };
	T = new int[powerTow[column] > row + 1 ? powerTow[column] : row + 1]{ 0 };
    fdRight = new int[powerTow[column]]{ 0 };
	/*for (int i = 0; i < powerTow[column]; i++) {
		fdRight[i] = 0;
	}*/
	fdLeftVis = new int[powerTow[column]]{0};
	S = new vector<int>[row + 1];
	L = new vector<int>[maxlevel];
	element = new vector<int>[powerTow[column]];
	for (int i = 0; i < powerTow[column]; i++) {
		int t = i, count = 0;
		while (t)
		{
			if (t & 1) {
				element[i].push_back(powerTow[count]);
			}
			count++;
			t = t >> 1;
		}
	}
	pi = new vector<vector<int>>[powerTow[column]];
	piStart = new unordered_map<string, vector<int>>[powerTow[column]];
}

TANE::~TANE()
{
	delete []piStart;
	delete []pi;
	delete []element;
	delete []L;
	delete []fdRight;
	delete []S;
	delete []powerTow;
	delete []cplus; 
	delete []levelIn;
	delete []T;
}

static bool LexicoCmp(const int & a, const int & b)
{
	int size = element[a].size() < element[b].size() ? element[a].size() : element[b].size();
	for (int i = 0; i < size; i++) {
		if (element[a][i] == element[b][i])continue;
		return element[a][i] < element[b][i];
	}
	return element[a].size() < element[b].size();
	return a < b;
}

void TANE::GetFunctionDependence()
{
	clock_t st, ed;
	level = 1;
	L[0].clear();
	cplus[0] = R;
	for (int i = 0; i < column; i++) {
		L[level].push_back(powerTow[i]);
		levelIn[powerTow[i]] = level;
	}
	StrippedInit();
	while (!L[level].empty()) {
		cout << level << endl;
		cout << L[level].size() << endl;
		st = clock();
		ComputeDependencies(level);
		Prune(level);
		GenerateNextLevel(level);
		level++;
		ed = clock();
		cout << (ed - st) * 1.0 / CLOCKS_PER_SEC << endl;
	}
}

void TANE::OutputFD()
{
	ofstream Outfile("result.txt");
	int lsize = fdLeft.size();
	sort(fdLeft.begin(), fdLeft.end(), LexicoCmp);
	for (int k = 0; k < lsize; k++) {
		int X = fdLeft[k], A = fdRight[X];
		int xsize = element[X].size(), asize = element[A].size();
		for (int i = 0; i < asize; i++) {
			for (int j = 0; j < xsize; j++) {
					Outfile << log2(element[X][j])+1 << " ";
			}
			Outfile << "-> ";
			Outfile << log2(element[A][i])+1 << endl;
		}
	}
	Outfile.close();
}

void TANE::StrippedInit()
{
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < column; j++) {
			piStart[j][table[i][j]].push_back(i + 1);
		}
	}
	unordered_map<string, vector<int>>::iterator iter;
	for (int i = 0; i < column; i++) {
		for (iter = piStart[i].begin(); iter != piStart[i].end(); iter++) {
			if (iter->second.size() >= 2) {
				pi[powerTow[i]].push_back(iter->second);
			}
		}
	}
}

void TANE::ComputeDependencies(int level)
{
	int lsize = l.size();
	for (int i = 0; i < lsize; i++) {
		int X = l[i], xsize = element[X].size(), end = R;
		for (int j = 0; j < xsize; j++) { // A
			end = end & cplus[X - element[X][j]];
		}
		cplus[X] = end;
	}
	for (int i = 0; i < lsize; i++) {
		int X = l[i], result = X & cplus[X], rsize = element[result].size();
		for (int j = 0; j < rsize; j++) {
			int A = element[result][j], XsubA = X - A;
			if (XsubA == 0)continue;
			int EX = GetEValueOfSingle(X), EXsubA = GetEValueOfSingle(XsubA);
			if (EXsubA - EX > eps)continue;
			if (EXsubA <= eps ||
				GetExactEValue(XsubA, A) <= eps) {//X\{A}->A is valid
												  //OutputFD(X - A, A);
				if (!fdLeftVis[XsubA]) {
					fdLeft.push_back(XsubA);
					fdLeftVis[XsubA] = 1;
				}
				fdRight[XsubA] = fdRight[XsubA] | A;
				cplus[X] = cplus[X] - A;
				if (EXsubA == EX) {
					int RsubX = R - X, rxsize = element[RsubX].size();
					for (int k = 0; k < rxsize; k++) {//B
						cplus[X] = cplus[X] - (cplus[X] & element[RsubX][k]);
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
			levelIn[X] = 0;
			continue;
		}
		if (Superkey(X)) {
			int result = cplus[X] - (cplus[X] & X);
			int xsize = element[X].size(), rsize = element[result].size();
			for (int j = 0; j < rsize; j++) {
				int A = element[result][j], end = R;
				for (int k = 0; k < xsize; k++) {
					int B = element[X][k];
					end = end & cplus[(X | A) - B];
				}
				if (A & end) {
					//output X->A
					if (!fdLeftVis[X]) {
						fdLeft.push_back(X);
						fdLeftVis[X] = 1;
					}
					fdRight[X] = fdRight[X] | A;
				}
			}
			//i = l.erase(i);
			//levelIn[X] = 0;
			//continue;
		}
		i++;
	}
}

void TANE::GenerateNextLevel(int level)
{
	int next = level + 1, lsize = l.size(), last = 0;
	int tsize, X;
	sort(L[level].begin(), L[level].end(), LexicoCmp);
	L[next].clear();
	tmp.push_back(l[last]);
	for (int i = 1; i < lsize; i++) {
		int same = l[i] & l[last];
		if (CountOne(same) == level - 1 && l[last] - same > same) {
			tmp.push_back(l[i]);
		}
		else {
			tsize = tmp.size();
			for (int j = 0; j < tsize; j++) { //Y
				for (int k = j + 1; k < tsize; k++) {//Z
					X = tmp[j] | tmp[k];
					StrippedProduct(tmp[j], tmp[k]);
					int flag = 1, xsize = element[X].size();
					for (int z = 0; z < xsize; z++) {//A
						if (levelIn[X - element[X][z]] != level) {
							flag = 0;
							break;
						}
					}
					if (flag && !levelIn[X]) {
						L[next].push_back(X);
						levelIn[X] = next;
					}
				}
			}
			tmp.clear();
			last = i;
			tmp.push_back(l[last]);
		}
	}
	tsize = tmp.size();
	for (int j = 0; j < tsize; j++) { //Y
		for (int k = j + 1; k < tsize; k++) {//Z
			X = tmp[j] | tmp[k];
			StrippedProduct(tmp[j], tmp[k]);
			int flag = 1, xsize = element[X].size();
			for (int z = 0; z < xsize; z++) {//A
				if (levelIn[X - element[X][z]] != level) {
					flag = 0;
					break;
				}
			}
			if (flag && !levelIn[X]) {
				L[next].push_back(X);
				levelIn[X] = next;
			}
		}
	}
	tmp.clear();
}

void TANE::StrippedProduct(int Y, int Z)
{
	int X = Y | Z;
	pi[X].clear();
	int ysize = pi[Y].size(), zsize = pi[Z].size();
	for (int i = 0; i < ysize; i++) {
		int suci = i + 1, size = pi[Y][i].size(); //ci
		for (int j = 0; j < size; j++) {
			int t = pi[Y][i][j];
			T[t] = suci;
		}
		S[suci].clear();
	}
	for (int i = 0; i < zsize; i++) {
		int t, size = pi[Z][i].size();
		for (int j = 0; j < size; j++) {
			t = pi[Z][i][j];
			if (T[t] != 0) {
				S[T[t]].push_back(t);
			}
		}
		for (int j = 0; j < size; j++) {
			t = pi[Z][i][j];
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
		int m = 1, csize = pi[X][i].size();
		for (int j = 0; j < csize; j++) {
			m = m >= T[pi[X][i][j]] ? m : T[pi[X][i][j]];
		}
		e = e + csize - m;
	}
	size = pi[XA].size();
	for (int i = 0; i < size; i++) {
		T[pi[XA][i][0]] = 0;
	}
	return e;
}

int TANE::GetEValueOfSingle(int X)
{
	int spi = 0, sspi = 0;
	spi = pi[X].size();
	for (int i = 0; i < spi; i++) {
		sspi += pi[X][i].size();
	}
	return (sspi - spi);
}

bool TANE::Superkey(int X)
{
	return pi[X].size() == 0;
}

int TANE::CountOne(int v)
{
	return element[v].size();
}

