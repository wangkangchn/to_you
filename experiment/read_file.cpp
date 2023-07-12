/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: read_file.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 读取文件
时间	   	: 2023-07-12 22:01
***************************************************************/
#include <iostream>  
#include <fstream>  
#include <vector>  
#include <stdio.h>
#include <string>
using namespace std;

int main(int argc, char *argv[])
{
    ifstream fin("test.txt");  /* 读文件 */
    std::string line;
    std::vector<std::string> lines;
    while (getline(fin, line)) {
        // cout << "ahha" << endl;
        std::cout << line << std::endl;
        lines.emplace_back(std::move(line));
    }
    // cout << "ahha1" << endl;

    for (auto it = lines.begin(); it != lines.end(); ++it) {
        if (it->find("script") != std::string::npos) {
            std::swap(*it, *(lines.end() - 1));
            break;
        }
    }

    std::cout << "\n\n替换后\n";
    for (auto& line : lines) {
        std::cout << line << std::endl;
    }
    fin.close();

    ofstream fout("test.txt.txt"); /* 写文件 */
    for (auto& line : lines) {
        fout << line << std::endl;
    }
    fout.close();
    return 0;
}
