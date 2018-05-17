#include "io_func.h"
#include "fd_func.h"
#include <iostream>
#include "TANE.h"
using namespace std;
vector<vector<string>> table;
int main() {
    string filePath;
    //cout << "Please input data file path: ";
    //cin >> filePath;
	filePath = "test_data.txt";
    readFromFile(filePath, table);
	TANE tane;
	tane.GetFunctionDependence();
	tane.OutputFD();
    //DFD(table);
    return 0;
}