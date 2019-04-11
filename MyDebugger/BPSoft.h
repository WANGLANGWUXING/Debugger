#pragma once
#include "BPObject.h"
#include "Expression.h"
class BPSoft:public BPObject
{
public:
	BPSoft(DbgObject& dbgObject,uaddr uAddr);
	virtual ~BPSoft();
	unsigned char m_uData;
public:
	// ����ϵ�
	virtual bool Install();
	// �Ƴ��ϵ�
	virtual bool Remove();
	// ���ر��ϵ������
	virtual E_BPType Type()const;
	// �ж϶ϵ��Ƿ����Լ�
	virtual bool IsMe(const EXCEPTION_DEBUG_INFO& exception)const;
};

