#include "BPTF.h"


BPTF::BPTF(DbgObject & dbgObj, bool bIsTFBP)
	:BPObject(dbgObj),m_bIsUserBP(bIsTFBP)
{
}

BPTF::~BPTF()
{
	Remove();
}

bool BPTF::Install()
{
	// 将TF标志位置为1即可
	// CONTEXT_INTEGER，查询通用数据寄存器
	// CONTEXT_CONTROL，查询控制寄存器组		
	CONTEXT ct = { CONTEXT_CONTROL };
	if (!m_dbgObj.GetRegInfo(ct))
		return false;
	PEFLAGS pEFlags = (PEFLAGS)&ct.EFlags;
	pEFlags->TF = 1;
	m_bOnce = m_condition.IsEmpty();
	return m_dbgObj.SetRegInfo(ct);
}

bool BPTF::NeetRemove() const
{
	return m_bOnce;
}

bool BPTF::Remove()
{
	return true;
}

bool BPTF::IsHit() const
{
	// 记录命中次数
	// 判断是否含有表达式
	if (!m_condition.IsEmpty()) {
		Expression exp(&m_dbgObj);
		return exp.GetValue(m_condition) != 0;
	}
	return m_bIsUserBP && m_bOnce;
}

E_BPType BPTF::Type() const
{
	return breakpointType_tf;
}

bool BPTF::IsMe(const EXCEPTION_DEBUG_INFO & exception) const
{
	// 判断异常类型是否匹配
	if (exception.ExceptionRecord.ExceptionCode != EXCEPTION_SINGLE_STEP)
		return false;
	//MessageBox(0, "单步断点", "断点提示", 0);

	// 获取调试寄存器
	
	CONTEXT ct = { CONTEXT_DEBUG_REGISTERS };
	if (!GetThreadContext(m_dbgObj.m_hCurrThread, &ct))
		return false;
	PDBG_REG6 pDr6 = (PDBG_REG6)&ct.Dr6;
	// 判断调试寄存器是否有值
	return !(pDr6->B0 || pDr6->B1 || pDr6->B2 || pDr6->B3);
}

void BPTF::ConvertToUserBreakpoint()
{
	Install();
	m_bIsUserBP = true;
}
