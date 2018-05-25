#include "io_func.h"

#include <iostream>
//#include <thread>
//#include <shared_mutex>
//#include <mutex>
#include <fstream>
#include <ctime>

#define THREAD_NUMBER 4

using namespace std;

void specialSplit(vector<string> &res, string &s) {
    // Source: http://www.cplusplus.com/faq/sequences/strings/split/
    res.clear();
    int current = 0, next = -1;
    int tempCur = 0; // tempCur to fix input bug
    int len = s.length();
    do
    {
        next = s.find(",", tempCur);
        tempCur = next + 1;
        if (next == string::npos || s[next + 1] != ' ') {
            res.push_back(s.substr(current, next - current));
            current = next + 1;
        }

    } while (next != string::npos);

}

void readFromFile(string filePath, vector<vector<string>> &table, bool isClear) {
    if (isClear)
        table.clear();

    ifstream fileIn;
    fileIn.open(filePath, ios::in);
    if (!fileIn.is_open()) {
        cout << "File open failed!" << endl;
        exit(1);
    }

    /*thread t[THREAD_NUMBER];
    vector<string> res[THREAD_NUMBER];
    for (int i = 0; i < THREAD_NUMBER; i++) {
        res[i] = vector<string>(20);
    }
    
    string s[THREAD_NUMBER];
    char buffer[THREAD_NUMBER][512];

    shared_mutex m;

    for (int th = 0; th < THREAD_NUMBER; th++) {
        t[th] = thread([&](int th) {
            while (!fileIn.eof()) {
                unique_lock<shared_mutex> write(m);
                fileIn.getline(buffer[th], 500);

                s[th] = string(buffer[th]);
                if (s[th] == "") {
                    write.unlock();
                    continue;
                }
                specialSplit(res[th], s[th]);
                
                
                table.push_back(res[th]);
                write.unlock();
            }
        }, th);
    }

    for (int th = 0; th < THREAD_NUMBER; th++) {
        t[th].join();
    }*/

    vector<string> res;
    string s;
    char buffer[512];
    
    while (!fileIn.eof()) {
        fileIn.getline(buffer, 500);
        s = string(buffer);
        if (s == "") {
            continue;
        }
        specialSplit(res, s);
        table.push_back(res);
    }

    fileIn.close();
}
