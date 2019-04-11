#include "BreakpointEngine.h"



BreakpointEngine::BreakpointEngine()
	:m_pRecoveryBp(0)
{
}


BreakpointEngine::~BreakpointEngine()
{
	Clear();
}
// 查找断点 返回list的一个迭代器
BpItr BreakpointEngine::FindBreakpoint(const EXCEPTION_DEBUG_INFO & exception)
{
	BPObject* pBp = nullptr;
	for (BpItr i = m_bpList.begin(); i !=m_bpList.end(); i++)
	{
		// 利用断点对象自己提供的方法来断点异常信息是否是由该断点产生的
		if ((*i)->IsMe(exception))
			return i;
	}
	return m_bpList.end();
}
// 修复异常
bool BreakpointEngine::FixException(BpItr findItr)
{
	BPObject* pBp = *findItr;
	// 从被调试进程中移除断点（使断点失效）
	pBp->Remove();
	// 获取断点是否被命中，内部会有一下条件表达式，
	// 如果表达式为true才被命中，否则不被命中
	bool bHit = pBp->IsHit();
	// 判断断点是否需要从断点列表中移除
	if (pBp->NeedRemove())
	{
		// 释放空间
		delete pBp;
		// 从断点列表中删除记录
		m_bpList.erase(findItr);
	}
	else // 没有被命中.
	{
		// tf断点身兼两职:
		//  1. 作为恢复其他功能断点而被设下的tf断点
		//  2. 用户单步时设下的tf断点.
		// 所以,如果tf断点重复,就意味着有一个功能断点需要修复,而且用户正好又要下单步断点
		// 在这种情况, 就不能简单地删除tf断点,也不能再次再插入一个tf断点.
		if (pBp->Type() == breakpointType_tf)
		{
			pBp->Install();
			return bHit;
		}

		// 因为断点已经被移除, 断点已经失效,因此,需要恢复断点的有效性
		// 将断点放入待恢复断点表中
		m_pRecoveryBp = pBp;

		// 插入tf断点,触发一个异常,用于恢复失效的断点
		BPObject *pTf = new BPTF(*this, false);
		pTf->Install();
		m_bpList.push_front(pTf);
	}

	// 返回断点是否被命中
	return bHit;
}
// 重新安装断点
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
		// 如果是一次性断点,即使类型相同也视为不一样的断点(此处存在逻辑隐患)

		if (i->GetAddress() == uAddress && i->m_bOnce != true)
		{
			return i;
		}
		
	}
	return nullptr;
}
// 根据提供的地址和类型查找断点,返回断点对象
BPObject * BreakpointEngine::FindBreakpoint(uaddr uAddress, E_BPType eType)
{
	for (auto&i:m_bpList)
	{
		if (i->GetAddress() == uAddress&&i->Type() == eType)
			return i;
	}
	return nullptr;
}
// 添加断点到断点列表中
BPObject * BreakpointEngine::AddBreakpoint(uaddr uAddress, E_BPType eType, uint uLen)
{
	// 添加断点
	BPObject	*pBp = nullptr;

	// 判断是否含有重复断点
	pBp = CheckRepeat(uAddress, eType);
	if (pBp != nullptr)
	{
		// 判断是否有重复的TF断点
		if (pBp->Type() != breakpointType_tf)
			return nullptr;

		// 如果重复的断点是TF断点,则需要将TF断点转换
		// 转换成用户断点（否则无法断在用户界面上）
		((BPTF*)pBp)->ConvertToUserBreakpoint();
		return pBp;
	}

	if (eType == breakpointType_tf)// TF断点
		pBp = new BPTF(*this, true);
	else if (eType == breakpointType_soft) // 软件断点
		pBp = (new BPSoft(*this, uAddress));
	else if (eType >= breakpointType_acc && eType <= breakpointType_acc_rw)//内存访问断点
		pBp = (new BPAcc(*this, uAddress, eType, uLen));
	else if (eType >= breakpointType_hard && eType <= breakpointType_hard_rw)// 硬件断点
		pBp = (new BPHard(*this, uAddress, eType, uLen));
	else
		return nullptr;
	if (pBp->Install() == false)
	{
		delete pBp;
		return false;
	}

	// 将断点插入到断点链表中
	m_bpList.push_front(pBp);
	return pBp;
}
// 添加断点 （通过Api名称添加）
BPObject * BreakpointEngine::AddBreakpoint(const char * pszApiName)
{
	// 查找API的地址
	uaddr address = FindApiAddress(m_hCurrProcess, pszApiName);
	if (address == 0)
		return nullptr;
	// 添加一个软件断点
	return AddBreakpoint(address, breakpointType_soft);
}
// 移除断点
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
// 给断点设置一个条件表达式
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

//  清空断点列表
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
