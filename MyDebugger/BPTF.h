#pragma once
#include "BPObject.h"
#include "RegStruct.h"
#include "Expression.h"
class BPTF:public BPObject
{
	uint m_bIsUserBP; // 记录是否是用户下的TF断点（单步断点）
public:
	BPTF(DbgObject& dbgObj,bool bIsTFBP = TRUE);
	virtual ~BPTF();
public:
	// 插入断点
	virtual bool Install();
	// 是否需要移除断点
	virtual bool NeetRemove()const;
	// 移除断点
	virtual bool Remove();
	// 判断断点是否被命中
	virtual bool IsHit() const;
	// 返回本断点的类型
	virtual E_BPType Type() const;
	// 判断断点是否是自己
	virtual bool IsMe(const EXCEPTION_DEBUG_INFO& exception)const;
	// 转换为用户断点
	void ConvertToUserBreakpoint();
};

