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
	int R; //�������� = 2^column - 1
	int *cplus; //rhs+
	int *shift; //2^i
	int *T;
	int *vis; //�ж�L���Ƿ��ظ�����ĳ����
	int *result_vis;
	vector<int>*L; //��int����ʾ����,��iλ��01������i�������Ƿ��ڼ�����
	vector<int>*S;
	vector<int>tmp;
	vector<int>result_left;
	vector<int>*result_right;//��������Ҷ�,ͨ����˲���
	unordered_map<string, vector<int>>*init_pi; //һ�����Եĵȼ���
	vector<vector<int>>*pi; //stripped �ȼ���
	void ComputeDependencies(int level);
	void Prune(int level);
	void GenerateNextLevel(int level);
	void StrippedInit();
	void StrippedProduct(int Y,int Z);
	int GetExactEValue(int X, int A); //E(X) = e(x)*row ��֤Ϊ���� 
	int GetEValueOfSingle(int X);
	bool Superkey(int X);
};

