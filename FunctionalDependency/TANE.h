#pragma once
#include <vector>
#include <map>
#include <string>
using namespace std;
class TANE
{
public:
	TANE(int row, int attribute);
	~TANE();
	void GetFunctionDependence();
private:
	int max_level, level;
	int column, row;
	int R;
	int *cplus;
	int *result;
	int *shift;
	int *T;
	double **E;
	vector<int>*L; //第i位的01决定第i个属性是否在集合类
	vector<int>*S;
	vector<int>tmp;
	map<string, vector<int>>*init_pi;
	vector<vector<int>>*pi;

	void ComputeDependencies(int level);
	void Prune(int level);
	void GenerateNextLevel(int level);
	void StrippedInit();
	void StrippedProduct(int Y,int Z);
	double GetExactEValue(int X, int A);
	double GetEValueOfSingle(int X);
	bool Superkey(int X);
};

