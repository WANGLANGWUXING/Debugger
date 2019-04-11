#pragma once
#include "DbgObject.h"
#include "BPTF.h"
#include "DbgSymbol.h"
#include "BPSoft.h"
#include "BPHard.h"
#include "BPAcc.h"
#include <list>
using std::list;
// �ض������� ��������
typedef list<BPObject*>::iterator BpItr; 

class BreakpointEngine:public DbgObject,public DbgSymbol
{
	list<BPObject*> m_bpList;		// �ϵ��б�
	BPObject*		m_pRecoveryBp;	// ���ظ��б�
public:
	BreakpointEngine();
	~BreakpointEngine();
protected:
	// �����쳣��Ϣ�����Ҷϵ㣬���ضϵ���list �еĵ�����
	BpItr FindBreakpoint(const EXCEPTION_DEBUG_INFO& exception);
	// �޸��쳣
	bool FixException(BpItr findItr);
	// �ָ�ʧЧ�Ķϵ�
	bool ReInstallBreakpoint();
	// ���ϵ��Ƿ��ظ�
	BPObject* CheckRepeat(uaddr uAddress, E_BPType eType);
public:
	// ���ݵ�ַ�����Ͳ���һ���ϵ�
	BPObject* FindBreakpoint(uaddr uAddress, E_BPType eType = e_bt_none);
	// ���һ���ϵ㵽�ϵ��б��ϵ��ַ���ϵ����ͣ��ϵ����ݳ��ȣ�һ��ֻ����Ӳ���ϵ���ڴ���ʶϵ㣩��
	BPObject* AddBreakpoint(uaddr uAddress, E_BPType eType, uint uLen = 0);
	// ���һ���ϵ㵽�ϵ��б�ָ��һ��API�������ִ�Сд�����ϵ�ֻ������Ϊ����ϵ㣩
	BPObject* AddBreakpoint(const char* pszApiName);
	// �Ƴ�һ���ϵ㣨uIndex�Ƕϵ��ڶϵ��б��е�λ�ã�
	bool DeleteBreakpoint(uint uIndex);
	// �ж�һ���ϵ�������Ƿ�����Ч��
	bool IsInvalidIterator(const BpItr& itr)const;
	// ��ȡ�ϵ��б�Ŀ�ʼ������
	list<BPObject*>::const_iterator GetBPListBegin() const;
	// ��ȡ�ϵ��б�Ľ���������
	list<BPObject*>::const_iterator GetBPListEnd()const;
	// ���ϵ�����һ���������ʽ
	static void SetExp(BPObject* pBp, const CStringA& strExp);
	// ���ϵ����ó�һ���Զϵ�
	static void SetOnce(BPObject* pBp, bool bOnce);
	// ��նϵ��б�
	void Clear();
};

