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
	// ��ʼ������
	void InitSymbol(HANDLE hProcess);
	// ���Һ�������Ӧ�ĵ�ַ
	SIZE_T FindApiAddress(HANDLE hProcess, const char*pszName);
	// ���ҵ�ַ��Ӧ�ĺ�����
	BOOL GetFunctionName(HANDLE hProcess, SIZE_T nAddress, CString& strName);
};

