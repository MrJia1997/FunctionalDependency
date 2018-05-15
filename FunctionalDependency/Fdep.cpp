#include "fd_func.h"
#include <iostream>
#include <unordered_set>
#include <algorithm>

using namespace std;

vector<string> DFD(vector<vector<string>> &table) {
    // TODO: partition function

    vector<string> res;
    int row = table.size();
    if (row == 0)
        return res;
    int col = table[0].size();
    
    unsigned int tempR = 0;
    if (col < 32)
        tempR = (1 << col) - 1;
    else {
        cout << "Only support columns less than 32" << endl;
        exit(1);
    }

    for (int i = 0; i < col; i++) {
        /* if (partition[i].size() == row)*/
        tempR ^= (1 << i);
        for (int j = 0; j < col; j++) {
            if ((tempR >> j) & 1) {
                res.push_back(to_string(i + 1) + " -> " + to_string(j + 1));
            }
        }
    }

    return res;
}