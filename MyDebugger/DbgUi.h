#pragma once
#include <Windows.h>
#include <atlstr.h>
#include <stdio.h>
#include <list>
#include "BreakpointEngine.h"
#include "hightLight.h"
#include "RegStruct.h"
using std::list;

#define SHOW_TEXT_HEX		0x000000001
#define SHOW_TEXT_DEC		0x000000002
#define SHOW_LEN_BYTE		0x000000004
#define SHOW_LEN_WORD		0x000000008
#define SHOW_LEN_DWORD		0x000000010
#define SHOW_LEN_QWORD		0x000000020
#define	SHOW_TEXT_ANSI		0x000000040
#define	SHOW_TEXT_UNICODE	0x000000080

typedef enum
{
	e_st_log = 0,
	e_st_regInfo,
	e_st_memInfo,
	e_st_DisAsmInfo
}E_ShowType;

class DbgUi
{
	static HANDLE	m_hStdOut;
	BreakpointEngine* m_pBpEngine;
public:
	DbgUi(BreakpointEngine* pBpEngine);
	~DbgUi();
	inline void PrintReg(SIZE_T dwReg, WORD color);
	inline void PrintEflag(DWORD dwFlag, WORD color);
public:
	void ShowAsm(SIZE_T Addr,
		const char* ShowOpc,
		const char* pszDiAsm,
		const char* pszCom,
		const char* pszLineHeader = "  "
	);

	void ShowReg(const CONTEXT& ct);
	void ShowMem(SIZE_T virtualAddress, const LPBYTE lpBuff, int nSize, DWORD dwShowFlag);
	void ShowStack(SIZE_T virtualAddress, const LPBYTE lpBuff, int nSize);

	void ShowBreakPointList(list<BPObject*>::const_iterator beginItr, list<BPObject*>::const_iterator endItr);

};

