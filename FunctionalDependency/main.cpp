#include "io_func.h"
#include <iostream>
#include <ctime>
#include "TANE.h"
using namespace std;
vector<vector<string>> table(100000);

int main() {
    string filePath;
	clock_t st, ed;
    cout << "Please input data file path: ";
    cin >> filePath;
	st = clock();
    // filePath = "data.txt";
    readFromFile(filePath, table);
	cout << "start" << endl;
	TANE tane;
	tane.GetFunctionDependence();
	tane.OutputFD();
	ed = clock();
	cout << (ed - st) * 1.0 / CLOCKS_PER_SEC << endl;
    system("pause");
    return 0;
}