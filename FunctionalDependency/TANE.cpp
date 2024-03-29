#include "TANE.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <cmath>
#include <ctime>
using namespace std;
extern vector<vector<string>> table;
// vector<bool> lVisited;

#define l L[level]
#define eps 0
#define THREAD_NUMBER 4
vector<int> *element; //element of bitset i 
thread t[THREAD_NUMBER];

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
	cplusVis = new int[powerTow[column]]{ 0 };
    levelIn = new int[powerTow[column]]{ 0 };

    int tsize = powerTow[column] > row + 1 ? powerTow[column] : row + 1;
    T = new int*[THREAD_NUMBER];
    S = new vector<int>*[THREAD_NUMBER];
    for (int i = 0; i < THREAD_NUMBER; i++) {
        T[i] = new int[tsize]{ 0 };
        S[i] = new vector<int>[row + 1];
    }
    
    fdRight = new int[powerTow[column]]{ 0 };
    fdLeftVis = new int[powerTow[column]]{ 0 };
    L = new vector<int>[maxlevel];
    element = new vector<int>[powerTow[column]];
    pi = new vector<vector<int>>[powerTow[column]];
    piStart = new unordered_map<string, vector<int>>[powerTow[column]];
    // Try to use multithread
    for (int th = 0; th < THREAD_NUMBER; th++) {
        t[th] = thread([&](int s, int e) {
            for (int i = s; i < e; i++) {
                int temp = i, count = 0;
                while (temp)
                {
                    if (temp & 1) {
                        element[i].push_back(powerTow[count]);
                    }
                    count++;
                    temp >>= 1;
                }
            }
        }, powerTow[column] * th / THREAD_NUMBER, powerTow[column] * (th + 1) / THREAD_NUMBER);
    }
    for (int i = 0; i < THREAD_NUMBER; i++) {
        t[i].join();
    }
}

TANE::~TANE()
{
	delete []piStart;
	delete []pi;
	delete []element;
	delete []L;
	delete []fdRight;
	delete []powerTow;
	delete []cplus; 
	delete []levelIn;
	
    for (int i = 0; i < 4; i++) {
        delete []T[i];
        delete []S[i];
    }
    delete []T;
    delete []S;

	delete []cplusVis;
}

struct LexicoCmp {
    bool operator() (const int & a, const int & b)
    {
        int size = element[a].size() < element[b].size() ? element[a].size() : element[b].size();
        for (int i = 0; i < size; i++) {
        	if (element[a][i] == element[b][i]) continue;
        	return element[a][i] < element[b][i];
        }
        return element[a].size() < element[b].size();
    }
};

void TANE::GetFunctionDependence()
{
	level = 1;
	L[0].clear();
	cplus[0] = R;
	for (int i = 0; i < column; i++) {
		L[level].push_back(powerTow[i]);
		levelIn[powerTow[i]] = level;
	}
	StrippedInit();
	while (!L[level].empty()) {
        //cout << level << " " << L[level].size() << endl;
		//st = clock();
		ComputeDependencies(level);
		Prune(level);
		GenerateNextLevel(level);
		level++;
        //ed = clock();
		//cout << (ed - st) * 1.0 / CLOCKS_PER_SEC << endl;
	}
}

void TANE::OutputFD()
{
	ofstream Outfile("result.txt");
	int lsize = fdLeft.size();
	std::sort(fdLeft.begin(), fdLeft.end(), LexicoCmp());
	for (int k = 0; k < lsize; k++) {
		int X = fdLeft[k], A = fdRight[X];
		int xsize = element[X].size(), asize = element[A].size();
		for (auto &a : element[A]) {
			for (auto &x : element[X]) {
					Outfile << log2(x)+1 << " ";
			}
			Outfile << "-> ";
			Outfile << log2(a)+1 << endl;
		}
	}
	Outfile.close();
}

void TANE::StrippedInit()
{
    // The order in pi should be guaranteed
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
		cplusVis[X] = 1;
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
				int A = element[result][j];
				for (int k = 0; k < xsize; k++) {
					int B = element[X][k], C = (X | A) - B;
					if (!cplusVis[C])A = A & GetCPlus(C);
					else A = A & cplus[(X | A) - B];
				}
				if (A) {
					//output X->A
					if (!fdLeftVis[X]) {
						fdLeft.push_back(X);
						fdLeftVis[X] = 1;
					}
					fdRight[X] = fdRight[X] | A;
				}
			}
			i = l.erase(i);
			levelIn[X] = 0;
			continue;
		}
		i++;
	}
}

void TANE::GenerateNextLevel(int level)
{
	int next = level + 1, lsize = l.size(), last = 0;
	int tsize;
	// sort(l.begin(), l.end(), LexicoCmp());
	L[next].clear();
    tmp.push_back(l[last]);
    
    vector<int> tempL[THREAD_NUMBER];

    for (int i = 1; i < lsize; i++) {
		int same = l[i] & l[last];
		if (CountOne(same) == level - 1 && l[last] - same > same) {
			tmp.push_back(l[i]);
		}
		else {
			tsize = tmp.size();
            
            for (int th = 0; th < THREAD_NUMBER; th++) {
                t[th] = thread([&, this](int st, int ed, int th) {
                    for (int j = st; j < ed; j++) { //Y
                        for (int k = j + 1; k < tsize; k++) {//Z
                            int X = tmp[j] | tmp[k];
                            int flag = 1, xsize = element[X].size();
                            for (int z = 0; z < xsize; z++) {//A
                                if (levelIn[X - element[X][z]] != level) {
                                    flag = 0;
                                    break;
                                }
                            }
                            if (flag && !levelIn[X]) {
                                tempL[th].push_back(X);
                                levelIn[X] = next;
                                StrippedProduct(tmp[j], tmp[k], th);
                            }
                        }
                    }
                }, tsize * (th * th) / (THREAD_NUMBER * THREAD_NUMBER),
                    tsize * (th + 1) * (th + 1) / (THREAD_NUMBER * THREAD_NUMBER),
                    th);
            }

            for (int th = 0; th < THREAD_NUMBER; th++) {
                t[th].join();
            }

            //for (int j = 0; j < tsize; j++) { //Y
            //    for (int k = j + 1; k < tsize; k++) {//Z
            //        int X = tmp[j] | tmp[k];
            //        int flag = 1, xsize = element[X].size();
            //        for (int z = 0; z < xsize; z++) {//A
            //        	if (levelIn[X - element[X][z]] != level) {
            //        		flag = 0;
            //        		break;
            //        	}
            //        }
            //        if (flag && !levelIn[X]) {
            //        	L[next].push_back(X);
            //        	levelIn[X] = next;
            //        	StrippedProduct(tmp[j], tmp[k], 0);
            //        }
            //    }
            //}
			tmp.clear();
			last = i;
			tmp.push_back(l[last]);
		}
	}
    
	tsize = tmp.size();
    
    for (int th = 0; th < THREAD_NUMBER; th++) {
        t[th] = thread([&, this](int st, int ed, int th) {
            for (int j = st; j < ed; j++) { //Y
                for (int k = j + 1; k < tsize; k++) {//Z
                    int X = tmp[j] | tmp[k];
                    int flag = 1, xsize = element[X].size();
                    for (int z = 0; z < xsize; z++) {//A
                        if (levelIn[X - element[X][z]] != level) {
                            flag = 0;
                            break;
                        }
                    }
                    if (flag && !levelIn[X]) {
                        tempL[th].push_back(X);
                        levelIn[X] = next;
                        StrippedProduct(tmp[j], tmp[k], th);
                    }
                }
            }
        }, tsize * (th * th) / (THREAD_NUMBER * THREAD_NUMBER),
            tsize * (th + 1) * (th + 1) / (THREAD_NUMBER * THREAD_NUMBER),
            th);
    }

    for (int th = 0; th < THREAD_NUMBER; th++) {
        t[th].join();
    }


    // Concat tempL into L[next];
    for (int i = 0; i < THREAD_NUMBER; i++) {
        L[next].insert(
            L[next].end(),
            make_move_iterator(tempL[i].begin()),
            make_move_iterator(tempL[i].end())
        );
    }

    sort(L[next].begin(), L[next].end(), LexicoCmp());

	tmp.clear();
}

void TANE::StrippedProduct(int Y, int Z, int th)
{
    int X = Y | Z;
	if (pi[Z].size() < pi[Y].size()) swap(Y, Z);
	pi[X].clear();
	int ysize = pi[Y].size(), zsize = pi[Z].size();
	for (int i = 0; i < ysize; i++) {
		int suci = i + 1, size = pi[Y][i].size(); //ci
		for (int j = 0; j < size; j++) {
			T[th][pi[Y][i][j]] = suci;
		}
		S[th][suci].clear();
	}
	for (int i = 0; i < zsize; i++) {
		int t, size = pi[Z][i].size();
		for (int j = 0; j < size; j++) {
			t = pi[Z][i][j];
			if (T[th][t] != 0) {
				S[th][T[th][t]].push_back(t);
			}
		}
		for (int j = 0; j < size; j++) {
			t = pi[Z][i][j];
			if (S[th][T[th][t]].size() >= 2) {
				pi[X].push_back(S[th][T[th][t]]);
			}
			S[th][T[th][t]].clear();
		}
	}
	for (int i = 0; i < ysize; i++) {
		int size = pi[Y][i].size();
		for (int j = 0; j < size; j++) {
			T[th][pi[Y][i][j]] = 0;
		}
	}
}

int TANE::GetExactEValue(int X, int A)
{
    int e = 0, size;
	int XA = X | A;
	size = pi[XA].size();
	for (int i = 0; i < size; i++) {
		T[0][pi[XA][i][0]] = pi[XA][i].size();
	}
	size = pi[X].size();
	for (int i = 0; i < size; i++) {
		int m = 1, csize = pi[X][i].size();
		for (int j = 0; j < csize; j++) {
			m = m >= T[0][pi[X][i][j]] ? m : T[0][pi[X][i][j]];
		}
		e = e + csize - m;
	}
	size = pi[XA].size();
	for (int i = 0; i < size; i++) {
		T[0][pi[XA][i][0]] = 0;
	}

	return e;
}

int TANE::GetEValueOfSingle(int X)
{
	int spi = pi[X].size(), sspi = 0;
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

int TANE::GetCPlus(int X)
{
	int xsize = element[X].size(), end = R, result;
	for (int j = 0; j < xsize; j++) { // A
		result = X - element[X][j];
		if (cplusVis[result])end = end & cplus[result];
		else end = end & GetCPlus(X - element[X][j]);
	}
	cplus[X] = end;
	cplusVis[X] = 1;
	return cplus[X];
}

