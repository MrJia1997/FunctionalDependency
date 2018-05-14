#include "io_func.h"

#include <iostream>
#include <fstream>

using namespace std;

vector<string> split(const string &s, const char* delim = " ") {
    // Source: http://www.cplusplus.com/faq/sequences/strings/split/#string-find_first_of
    int current, next = -1;
    vector<string> res;
    do
    {
        current = next + 1;
        next = s.find_first_of(delim, current);
        res.push_back(s.substr(current, next - current));
    } while (next != string::npos);

    return res;
}

void readFromFile(string filePath, vector<vector<string>> &table, bool isClear) {
    if (isClear)
        table.clear();

    ifstream fileIn;
    fileIn.open(filePath, ios::in);
    if (!fileIn.is_open()){
        cout << "File open failed!" << endl;
        exit(1);
    }

    char buffer[256];
    while (!fileIn.eof()) {
        fileIn.getline(buffer, 150);
        string s(buffer);
        table.push_back(split(s, ","));
    }

    fileIn.close();
}
