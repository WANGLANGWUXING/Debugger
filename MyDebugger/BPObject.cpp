#include "BPObject.h"


BPObject::BPObject(DbgObject & dbgObj):m_dbgObj(dbgObj),m_bOnce()
{
}

BPObject::~BPObject()
{
}

uaddr BPObject::GetAddress() const
{
	return m_uAddress;
}

const char * BPObject::GetCondition() const
{
	if (m_condition.IsEmpty())
		return nullptr;
	return m_condition;
}

bool BPObject::IsHit() const
{
	if (!m_condition.IsEmpty())
	{
		Expression exp(&m_dbgObj);
		return exp.GetValue(m_condition) != 0;
	}
	return true;
}

bool BPObject::NeedRemove() const
{
	return m_bOnce;
}



void BPObject::SetCondition(const char * strCondition)
{
	m_condition = strCondition;
	m_bOnce = false;
}

void BPObject::SetCondition(bool bOnce)
{
	m_bOnce = bOnce;
	m_condition.Empty();
}
