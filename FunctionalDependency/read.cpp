#include "io_func.h"

#include <iostream>
#include <fstream>

using namespace std;

vector<string> specialSplit(const string &s, const char* delim = " ") {
    // Source: http://www.cplusplus.com/faq/sequences/strings/split/#string-find_first_of
    int current = 0, next = -1;
    int tempCur = 0; // tempCur to fix input bug
    vector<string> res;
    do
    {
        next = s.find_first_of(delim, tempCur);
        tempCur = next + 1;
        if (s[next + 1] != ' ') {
            res.push_back(s.substr(current, next - current));
            current = next + 1;
        }

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

    char buffer[512];
    while (!fileIn.eof()) {
        fileIn.getline(buffer, 500);
        string s(buffer);
        table.push_back(specialSplit(s, ","));
    }

    for (auto t : table) {
        if (t.size() > 15) {
            cout << t.size() << endl;
        }
    }

    fileIn.close();
}
