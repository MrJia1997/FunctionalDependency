#pragma once
#include <vector>
#include <unordered_map>
#include <string>
using namespace std;
class TANE
{
public:
	TANE();
	~TANE();
	void GetFunctionDependence();
	void OutputFD();
private:
	int row, column;
	int level, maxlevel;
	int R;
	int *powerTow; //powerTo w[i]2^i;
	int *cplus;
	int **T;
	int *levelIn; //attr set i in which level
	int *fdRight;
	int *fdLeftVis;
	int *cplusVis;
	vector<int> fdLeft;
    vector<int> *L;
	vector<int> **S;
	vector<int> tmp;
	vector<vector<int>> *pi;
	unordered_map<string, vector<int>> *piStart;
	void StrippedInit();
	void ComputeDependencies(int level);
	void Prune(int level);
	void GenerateNextLevel(int level);
	void StrippedProduct(int Y, int Z, int th);
	int GetExactEValue(int X, int A); //E(X->A) = e(x->a)*row 
	int GetEValueOfSingle(int X);
	bool Superkey(int X);
	int CountOne(int v);
	int GetCPlus(int X);
};


