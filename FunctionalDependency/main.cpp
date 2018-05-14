#include "io_func.h"
#include <iostream>

using namespace std;

int main() {
    string filePath;
    cout << "Please input data file path: ";
    cin >> filePath;
    vector<vector<string>> table;
    readFromFile(filePath, table);

    return 0;
}