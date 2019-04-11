#pragma once
#include "BPObject.h"
#include "RegStruct.h"
#include "Expression.h"
class BPHard:public BPObject
{
	uint     m_uLen;       // �жϳ���
	E_BPType m_eType;      // �ϵ�����
	uint     m_uDbgRegNum; // �ϵ����ĸ��Ĵ���
public:
	BPHard(DbgObject& dbgObj,uaddr uAddress,E_BPType eType,uint uLen);
	virtual ~BPHard();
	// ����ϵ�
	virtual bool Install();
	// �Ƴ��ϵ�
	virtual bool Remove();
	// ���ر��ϵ������
	virtual	E_BPType Type()const;
	// �ж϶ϵ��Ƿ����Լ�
	virtual bool IsMe(const EXCEPTION_DEBUG_INFO& ExcDebInf)const;
};

