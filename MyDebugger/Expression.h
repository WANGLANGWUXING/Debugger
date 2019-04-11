#pragma once
#include "DbgObject.h"
class Expression
{
	DbgObject *m_pDbgObj;
protected:
	// ��ȡ�����ڴ�
	SSIZE_T ReadProcessMemory(LPVOID lpAddr, DWORD dwSize);
	// ��ȡ�̼߳Ĵ�����ֵ
	bool ReadRegValue(const char* pReg, const char** pEnd, SSIZE_T& uRegValue);
	// д���̼߳Ĵ�����ֵ
	bool WriteRegValue(const char* pReg, const char** pEnd, SSIZE_T& uRegValue);

	// ��ȡ���ʽ���յ�ֵ
	bool GetValue(SSIZE_T& uValue, const char* pExpression, const char** pEnd, int nPriorty);

public:
	Expression(DbgObject* pDbgObj);
	~Expression();
	// ��ȡ���ʽ��ֵ
	SSIZE_T GetValue(const char* pExpression);
};

