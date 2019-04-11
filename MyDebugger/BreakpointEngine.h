#pragma once
#include "DbgObject.h"
#include "BPTF.h"
#include "DbgSymbol.h"
#include "BPSoft.h"
#include "BPHard.h"
#include "BPAcc.h"
#include <list>
using std::list;
// 重定义类型 ：迭代器
typedef list<BPObject*>::iterator BpItr; 

class BreakpointEngine:public DbgObject,public DbgSymbol
{
	list<BPObject*> m_bpList;		// 断点列表
	BPObject*		m_pRecoveryBp;	// 待回复列表
public:
	BreakpointEngine();
	~BreakpointEngine();
protected:
	// 根据异常信息来查找断点，返回断点在list 中的迭代器
	BpItr FindBreakpoint(const EXCEPTION_DEBUG_INFO& exception);
	// 修复异常
	bool FixException(BpItr findItr);
	// 恢复失效的断点
	bool ReInstallBreakpoint();
	// 检测断点是否重复
	BPObject* CheckRepeat(uaddr uAddress, E_BPType eType);
public:
	// 根据地址和类型查找一个断点
	BPObject* FindBreakpoint(uaddr uAddress, E_BPType eType = e_bt_none);
	// 添加一个断点到断点列表（断点地址，断点类型，断点数据长度（一般只用于硬件断点和内存访问断点））
	BPObject* AddBreakpoint(uaddr uAddress, E_BPType eType, uint uLen = 0);
	// 添加一个断点到断点列表（指定一个API名（区分大小写），断点只能设置为软件断点）
	BPObject* AddBreakpoint(const char* pszApiName);
	// 移除一个断点（uIndex是断点在断点列表中的位置）
	bool DeleteBreakpoint(uint uIndex);
	// 判断一个断点迭代器是否是无效的
	bool IsInvalidIterator(const BpItr& itr)const;
	// 获取断点列表的开始迭代器
	list<BPObject*>::const_iterator GetBPListBegin() const;
	// 获取断点列表的结束迭代器
	list<BPObject*>::const_iterator GetBPListEnd()const;
	// 给断点设置一个条件表达式
	static void SetExp(BPObject* pBp, const CStringA& strExp);
	// 将断点设置成一次性断点
	static void SetOnce(BPObject* pBp, bool bOnce);
	// 清空断点列表
	void Clear();
};

