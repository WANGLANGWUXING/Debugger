#pragma once
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib,"Dbghelp.lib")
#include <atlstr.h>
#include "DbgObject.h"
class DbgSymbol
{
public:
	DbgSymbol();
	virtual ~DbgSymbol();
	// 初始化符号
	void InitSymbol(HANDLE hProcess);
	// 查找函数名对应的地址
	SIZE_T FindApiAddress(HANDLE hProcess, const char*pszName);
	// 查找地址对应的函数名
	BOOL GetFunctionName(HANDLE hProcess, SIZE_T nAddress, CString& strName);
};

