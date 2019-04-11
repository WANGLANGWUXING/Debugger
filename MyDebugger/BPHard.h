#pragma once
#include "BPObject.h"
#include "RegStruct.h"
#include "Expression.h"
class BPHard:public BPObject
{
	uint     m_uLen;       // 中断长度
	E_BPType m_eType;      // 断点类型
	uint     m_uDbgRegNum; // 断点在哪个寄存器
public:
	BPHard(DbgObject& dbgObj,uaddr uAddress,E_BPType eType,uint uLen);
	virtual ~BPHard();
	// 插入断点
	virtual bool Install();
	// 移除断点
	virtual bool Remove();
	// 返回本断点的类型
	virtual	E_BPType Type()const;
	// 判断断点是否是自己
	virtual bool IsMe(const EXCEPTION_DEBUG_INFO& ExcDebInf)const;
};

