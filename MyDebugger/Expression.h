#pragma once
#include "DbgObject.h"
class Expression
{
	DbgObject *m_pDbgObj;
protected:
	// 读取进程内存
	SSIZE_T ReadProcessMemory(LPVOID lpAddr, DWORD dwSize);
	// 读取线程寄存器的值
	bool ReadRegValue(const char* pReg, const char** pEnd, SSIZE_T& uRegValue);
	// 写入线程寄存器的值
	bool WriteRegValue(const char* pReg, const char** pEnd, SSIZE_T& uRegValue);

	// 获取表达式最终的值
	bool GetValue(SSIZE_T& uValue, const char* pExpression, const char** pEnd, int nPriorty);

public:
	Expression(DbgObject* pDbgObj);
	~Expression();
	// 获取表达式的值
	SSIZE_T GetValue(const char* pExpression);
};

