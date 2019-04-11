#include "BreakpointEngine.h"



BreakpointEngine::BreakpointEngine()
	:m_pRecoveryBp(0)
{
}


BreakpointEngine::~BreakpointEngine()
{
	Clear();
}
// ���Ҷϵ� ����list��һ��������
BpItr BreakpointEngine::FindBreakpoint(const EXCEPTION_DEBUG_INFO & exception)
{
	BPObject* pBp = nullptr;
	for (BpItr i = m_bpList.begin(); i !=m_bpList.end(); i++)
	{
		// ���öϵ�����Լ��ṩ�ķ������ϵ��쳣��Ϣ�Ƿ����ɸöϵ������
		if ((*i)->IsMe(exception))
			return i;
	}
	return m_bpList.end();
}
// �޸��쳣
bool BreakpointEngine::FixException(BpItr findItr)
{
	BPObject* pBp = *findItr;
	// �ӱ����Խ������Ƴ��ϵ㣨ʹ�ϵ�ʧЧ��
	pBp->Remove();
	// ��ȡ�ϵ��Ƿ����У��ڲ�����һ���������ʽ��
	// ������ʽΪtrue�ű����У����򲻱�����
	bool bHit = pBp->IsHit();
	// �ж϶ϵ��Ƿ���Ҫ�Ӷϵ��б����Ƴ�
	if (pBp->NeedRemove())
	{
		// �ͷſռ�
		delete pBp;
		// �Ӷϵ��б���ɾ����¼
		m_bpList.erase(findItr);
	}
	else // û�б�����.
	{
		// tf�ϵ������ְ:
		//  1. ��Ϊ�ָ��������ܶϵ�������µ�tf�ϵ�
		//  2. �û�����ʱ���µ�tf�ϵ�.
		// ����,���tf�ϵ��ظ�,����ζ����һ�����ܶϵ���Ҫ�޸�,�����û�������Ҫ�µ����ϵ�
		// ���������, �Ͳ��ܼ򵥵�ɾ��tf�ϵ�,Ҳ�����ٴ��ٲ���һ��tf�ϵ�.
		if (pBp->Type() == breakpointType_tf)
		{
			pBp->Install();
			return bHit;
		}

		// ��Ϊ�ϵ��Ѿ����Ƴ�, �ϵ��Ѿ�ʧЧ,���,��Ҫ�ָ��ϵ����Ч��
		// ���ϵ������ָ��ϵ����
		m_pRecoveryBp = pBp;

		// ����tf�ϵ�,����һ���쳣,���ڻָ�ʧЧ�Ķϵ�
		BPObject *pTf = new BPTF(*this, false);
		pTf->Install();
		m_bpList.push_front(pTf);
	}

	// ���ضϵ��Ƿ�����
	return bHit;
}
// ���°�װ�ϵ�
bool BreakpointEngine::ReInstallBreakpoint()
{
	if (m_pRecoveryBp == nullptr)
		return false;
	m_pRecoveryBp->Install();
	m_pRecoveryBp = nullptr;
	return true;
}

BPObject * BreakpointEngine::CheckRepeat(uaddr uAddress, E_BPType eType)
{
	for (auto& i : m_bpList)
	{
		// �����һ���Զϵ�,��ʹ������ͬҲ��Ϊ��һ���Ķϵ�(�˴������߼�����)

		if (i->GetAddress() == uAddress && i->m_bOnce != true)
		{
			return i;
		}
		
	}
	return nullptr;
}
// �����ṩ�ĵ�ַ�����Ͳ��Ҷϵ�,���ضϵ����
BPObject * BreakpointEngine::FindBreakpoint(uaddr uAddress, E_BPType eType)
{
	for (auto&i:m_bpList)
	{
		if (i->GetAddress() == uAddress&&i->Type() == eType)
			return i;
	}
	return nullptr;
}
// ��Ӷϵ㵽�ϵ��б���
BPObject * BreakpointEngine::AddBreakpoint(uaddr uAddress, E_BPType eType, uint uLen)
{
	// ��Ӷϵ�
	BPObject	*pBp = nullptr;

	// �ж��Ƿ����ظ��ϵ�
	pBp = CheckRepeat(uAddress, eType);
	if (pBp != nullptr)
	{
		// �ж��Ƿ����ظ���TF�ϵ�
		if (pBp->Type() != breakpointType_tf)
			return nullptr;

		// ����ظ��Ķϵ���TF�ϵ�,����Ҫ��TF�ϵ�ת��
		// ת�����û��ϵ㣨�����޷������û������ϣ�
		((BPTF*)pBp)->ConvertToUserBreakpoint();
		return pBp;
	}

	if (eType == breakpointType_tf)// TF�ϵ�
		pBp = new BPTF(*this, true);
	else if (eType == breakpointType_soft) // ����ϵ�
		pBp = (new BPSoft(*this, uAddress));
	else if (eType >= breakpointType_acc && eType <= breakpointType_acc_rw)//�ڴ���ʶϵ�
		pBp = (new BPAcc(*this, uAddress, eType, uLen));
	else if (eType >= breakpointType_hard && eType <= breakpointType_hard_rw)// Ӳ���ϵ�
		pBp = (new BPHard(*this, uAddress, eType, uLen));
	else
		return nullptr;
	if (pBp->Install() == false)
	{
		delete pBp;
		return false;
	}

	// ���ϵ���뵽�ϵ�������
	m_bpList.push_front(pBp);
	return pBp;
}
// ��Ӷϵ� ��ͨ��Api������ӣ�
BPObject * BreakpointEngine::AddBreakpoint(const char * pszApiName)
{
	// ����API�ĵ�ַ
	uaddr address = FindApiAddress(m_hCurrProcess, pszApiName);
	if (address == 0)
		return nullptr;
	// ���һ������ϵ�
	return AddBreakpoint(address, breakpointType_soft);
}
// �Ƴ��ϵ�
bool BreakpointEngine::DeleteBreakpoint(uint uIndex)
{
	if (uIndex >= m_bpList.size())
		return false;

	for (BpItr i = m_bpList.begin(); i != m_bpList.end(); ++i)
	{
		if (uIndex-- == 0)
		{
			if (m_pRecoveryBp == *i)
				m_pRecoveryBp = nullptr;
			if ((*i)->Type() == breakpointType_soft)
			{

			}
			//(*i)->Remove();
			delete *i;
			m_bpList.erase(i);
			return true;
		}
	}
	return false;
}

bool BreakpointEngine::IsInvalidIterator(const BpItr & itr) const
{
	return itr == m_bpList.end();
}

list<BPObject*>::const_iterator BreakpointEngine::GetBPListBegin() const
{
	return m_bpList.begin();
}

list<BPObject*>::const_iterator BreakpointEngine::GetBPListEnd() const
{
	return m_bpList.end();
}
// ���ϵ�����һ���������ʽ
void BreakpointEngine::SetExp(BPObject * pBp, const CStringA & strExp)
{
	if (pBp != nullptr)
		pBp->SetCondition(strExp);
}

void BreakpointEngine::SetOnce(BPObject * pBp, bool bOnce)
{
	if (pBp != nullptr)
		pBp->SetCondition(bOnce);
}

//  ��նϵ��б�
void BreakpointEngine::Clear()
{
	for (auto& i : m_bpList)
	{
		i->Remove();
		delete i;
	}
	m_bpList.clear();
	m_pRecoveryBp = nullptr;
}
