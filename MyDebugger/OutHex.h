#pragma once
#include <Windows.h>
#include <string>
#define MAX 16*10000
using namespace std;

//获取文件大小
DWORD FileSize();
//内存数据以十六进制导出到文件
void HexToFile();
//pe内容复制
void ExeTofile();

//内存数据dump到文件
void dump(string str);

