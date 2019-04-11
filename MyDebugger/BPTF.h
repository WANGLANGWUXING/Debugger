#pragma once
#include "BPObject.h"
#include "RegStruct.h"
#include "Expression.h"
class BPTF:public BPObject
{
	uint m_bIsUserBP; // ��¼�Ƿ����û��µ�TF�ϵ㣨�����ϵ㣩
public:
	BPTF(DbgObject& dbgObj,bool bIsTFBP = TRUE);
	virtual ~BPTF();
public:
	// ����ϵ�
	virtual bool Install();
	// �Ƿ���Ҫ�Ƴ��ϵ�
	virtual bool NeetRemove()const;
	// �Ƴ��ϵ�
	virtual bool Remove();
	// �ж϶ϵ��Ƿ�����
	virtual bool IsHit() const;
	// ���ر��ϵ������
	virtual E_BPType Type() const;
	// �ж϶ϵ��Ƿ����Լ�
	virtual bool IsMe(const EXCEPTION_DEBUG_INFO& exception)const;
	// ת��Ϊ�û��ϵ�
	void ConvertToUserBreakpoint();
};

