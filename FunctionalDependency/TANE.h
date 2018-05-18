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
	int max_level, level;
	int column, row;
	int R; //所有属性 = 2^column - 1
	int *cplus; //rhs+
	int *shift; //2^i
	int *T;
	int *vis; //判断L中是否重复出现某集合
	int *result_vis;
	vector<int>*L; //用int来表示集合,第i位的01决定第i个属性是否在集合内
	vector<int>*S;
	vector<int>tmp;
	vector<int>result_left;
	vector<int>*result_right;//结果储存右端,通过左端查找
	unordered_map<string, vector<int>>*init_pi; //一个属性的等价类
	vector<vector<int>>*pi; //stripped 等价类
	void ComputeDependencies(int level);
	void Prune(int level);
	void GenerateNextLevel(int level);
	void StrippedInit();
	void StrippedProduct(int Y,int Z);
	int GetExactEValue(int X, int A); //E(X) = e(x)*row 保证为整数 
	int GetEValueOfSingle(int X);
	bool Superkey(int X);
};

