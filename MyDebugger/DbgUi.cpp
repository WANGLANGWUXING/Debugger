#include "DbgUi.h"
HANDLE DbgUi::m_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

DbgUi::DbgUi(BreakpointEngine * pBpEngine)
	:m_pBpEngine(pBpEngine)
{
}

DbgUi::~DbgUi()
{
}



inline void DbgUi::PrintReg(SIZE_T dwReg, WORD color)
{
	SetConsoleTextAttribute(m_hStdOut, color);
	printf(" %08X", dwReg);
	SetConsoleTextAttribute(m_hStdOut, F_WHITE);
	printf(" |");
}

inline void DbgUi::PrintEflag(DWORD dwFlag, WORD color)
{
	SetConsoleTextAttribute(m_hStdOut, color);
	printf("%3d ", dwFlag);
	SetConsoleTextAttribute(m_hStdOut, F_WHITE);
	printf("|");
}

void DbgUi::ShowAsm(SIZE_T Addr, const char * ShowOpc, const char * pszDiAsm, const char * pszCom, const char * pszLineHeader)
{
	//如果地址是断点 显示断点的颜色
	if (nullptr != m_pBpEngine->FindBreakpoint(Addr, breakpointType_soft))
	{
		SetConsoleTextAttribute(m_hStdOut, B_H_RED);//红色
		printf("%s0x%08X", pszLineHeader, Addr);
	}
	else
	{
		printf("%s0x%08X", pszLineHeader, Addr);
	}

	//输出OPCODE
	SetConsoleTextAttribute(m_hStdOut, F_WHITE);//恢复默认颜色
	printf(" | %24s | ", ShowOpc);

	//printf("  %08X | %-24s | ", Addr, ShowOpc);
	SIZE_T nLen = strlen(pszDiAsm);
	//开始高亮输出反汇编代码代码
	for (SIZE_T i = 0; i < nLen; i++)
	{
		SetConsoleTextAttribute(m_hStdOut, F_WHITE);		//默认的白色
															//跳转高亮（J）
		if (pszDiAsm[i] == 'j')
		{
			if (pszDiAsm[i + 1] == 'm'&&pszDiAsm[i + 2] == 'p')
			{
				SetConsoleTextAttribute(m_hStdOut, B_H_GREEN);//绿色
				while (pszDiAsm[i] != ' '&&pszDiAsm[i] != '\t')
				{
					printf("%c", pszDiAsm[i]);
					i++;
				}
				i--;
				continue;
			}
			else
			{
				SetConsoleTextAttribute(m_hStdOut, B_H_YELLOW);//黄色
				while (pszDiAsm[i] != ' '&&pszDiAsm[i] != '\t')
				{
					printf("%c", pszDiAsm[i]);
					i++;
				}
				i--;
				continue;
			}
		}
		//CALL高亮
		if (pszDiAsm[i] == 'c'&&pszDiAsm[i + 1] == 'a'&&pszDiAsm[i + 2] == 'l'&&pszDiAsm[i + 3] == 'l')
		{
			SetConsoleTextAttribute(m_hStdOut, B_H_RED);//红色
			while (pszDiAsm[i] != ' '&&pszDiAsm[i] != '\t')
			{
				printf("%c", pszDiAsm[i]);
				i++;
			}
			i--;
			continue;
		}
		//RET高亮
		if ((pszDiAsm[i] == 'r'&&pszDiAsm[i + 1] == 'e'&&pszDiAsm[i + 2] == 't') ||
			(pszDiAsm[i] == 'i'&&pszDiAsm[i + 1] == 'r'&&pszDiAsm[i + 2] == 'e'))
		{
			SetConsoleTextAttribute(m_hStdOut, F_H_LIGHTBLUE  /* |REVERSE */);//青色
			while (pszDiAsm[i] != ' '&&pszDiAsm[i] != '\t')
			{
				printf("%c", pszDiAsm[i]);
				i++;
			}
			i--;
			continue;
		}
		//PUSH POP高亮
		if ((pszDiAsm[i] == 'p'&&pszDiAsm[i + 1] == 'u'&&pszDiAsm[i + 2] == 's') ||
			(pszDiAsm[i] == 'p'&&pszDiAsm[i + 1] == 'o'&&pszDiAsm[i + 2] == 'p'))
		{
			SetConsoleTextAttribute(m_hStdOut, F_H_PURPLE);//带选择！！！
			while (pszDiAsm[i] != ' '&&pszDiAsm[i] != '\t')
			{
				printf("%c", pszDiAsm[i]);
				i++;
			}
			i--;
			continue;
		}
		//立即数高亮
		if ((pszDiAsm[i + 8] == 'h') && (pszDiAsm[i - 1] == ' ' || pszDiAsm[i - 1] == ',')
			&& i > 5)
		{
			SetConsoleTextAttribute(m_hStdOut, F_H_YELLOW);//黄色
			while (pszDiAsm[i] != ' '&&pszDiAsm[i] != '\0')
			{
				printf("%c", pszDiAsm[i]);
				i++;
			}
			i--;
			continue;
		}
		//内存地址高亮
		if ((pszDiAsm[i] == 'b'&&pszDiAsm[i + 1] == 'y'&&pszDiAsm[i + 2] == 't'
			&&pszDiAsm[i + 3] == 'e') ||
			(pszDiAsm[i] == 'w'&&pszDiAsm[i + 1] == 'o'&&pszDiAsm[i + 2] == 'r'
				&&pszDiAsm[i + 3] == 'd') ||
				(pszDiAsm[i] == 'd'&&pszDiAsm[i + 1] == 'w'&&pszDiAsm[i + 2] == 'o'
					&&pszDiAsm[i + 3] == 'r'&&pszDiAsm[i + 4] == 'd') ||
					(pszDiAsm[i] == 'f'&&pszDiAsm[i + 1] == 'w'&&pszDiAsm[i + 2] == 'o'
						&&pszDiAsm[i + 3] == 'r'&&pszDiAsm[i + 4] == 'd') ||
						(pszDiAsm[i] == 'q'&&pszDiAsm[i + 1] == 'w'&&pszDiAsm[i + 2] == 'o'
							&&pszDiAsm[i + 3] == 'r'&&pszDiAsm[i + 4] == 'd'))
		{
			if ((pszDiAsm[i - 2] == 'l'&&pszDiAsm[i - 3] == 'l') &&
				(pszDiAsm[i - 4] == 'a'&&pszDiAsm[i - 5] == 'c'))
			{
				//CALL地址高亮
				SetConsoleTextAttribute(m_hStdOut, F_H_PURPLE);
				while (pszDiAsm[i] != ',' && pszDiAsm[i] != '\0')
				{
					printf("%c", pszDiAsm[i]);
					i++;
				}
				i--;
				continue;
			}
			else
			{
				//DS高亮
				SetConsoleTextAttribute(m_hStdOut, F_H_LIGHTBLUE);
				while (pszDiAsm[i] != ',' && pszDiAsm[i] != '\0')
				{
					printf("%c", pszDiAsm[i]);
					i++;
				}
				i--;
				continue;
			}
		}
		//内存地址立即数高亮
		if ((pszDiAsm[i - 6] == 'j' || pszDiAsm[i - 4] == 'j' || pszDiAsm[i - 3] == 'j') ||
			(pszDiAsm[i - 5] == 'c'&&pszDiAsm[i - 4] == 'a' &&pszDiAsm[i - 3] == 'l' &&pszDiAsm[i - 2] == 'l') ||
			(pszDiAsm[i - 5] == 'l'&& pszDiAsm[i - 4] == 'o'&&pszDiAsm[i - 3] == 'o'&&pszDiAsm[i - 1] == '\t') ||
			(pszDiAsm[i - 6] == 'l'&&pszDiAsm[i - 5] == 'o'&&pszDiAsm[i - 4] == 'o'&&pszDiAsm[i - 1] == '\t') ||
			(pszDiAsm[i - 7] == 'l'&&pszDiAsm[i - 6] == 'o'&&pszDiAsm[i - 5] == 'o'&&pszDiAsm[i - 1] == '\t'))
		{
			SetConsoleTextAttribute(m_hStdOut, F_H_GREEN);//绿色
			while (pszDiAsm[i] != ',' && pszDiAsm[i] != '\0')
			{
				printf("%c", pszDiAsm[i]);
				i++;
			}
			i--;
			continue;
		}
		SetConsoleTextAttribute(m_hStdOut, F_H_WHITE);
		printf("%c", pszDiAsm[i]);//输出没有高亮的字符
	}

	//if (pszCom != NULL)
	//{
	//	SetConsoleTextAttribute(m_hStdOut, F_H_GREEN);//绿色
	//	printf("%*c %s", 40 - strlen(pszDiAsm), '|', pszCom);
	//}
	SetConsoleTextAttribute(m_hStdOut, F_WHITE);//恢复默认颜色
	printf("\n");//输出换行
}

void DbgUi::ShowReg(const CONTEXT & ct)
{


	static	bool bFirst = true;
	static	CONTEXT		oldCt = { 0 };
	if (bFirst)
	{
		bFirst = false;
		oldCt = ct;
	}

	printf("------------------------------------------------------------------\n");
	printf("    eax   |    ecx   |    edx   |    ebx   |    esi   |    edi   |\n");
	// 	printf(" %08X | %08X | %08X | %08X | %08X | %08X |\n" , 
	// 		   ct.Eax , ct.Ecx , ct.Edx , ct.Ebx , ct.Esi , ct.Edi);
	if (oldCt.Eax != ct.Eax)
		PrintReg(ct.Eax, F_H_RED);
	else
		PrintReg(ct.Eax, F_WHITE);
	if (oldCt.Ecx != ct.Ecx)
		PrintReg(ct.Ecx, F_H_RED);
	else
		PrintReg(ct.Ecx, F_WHITE);

	if (oldCt.Edx != ct.Edx)
		PrintReg(ct.Edx, F_H_RED);
	else
		PrintReg(ct.Edx, F_WHITE);

	if (oldCt.Ebx != ct.Ebx)
		PrintReg(ct.Ebx, F_H_RED);
	else
		PrintReg(ct.Ebx, F_WHITE);

	if (oldCt.Esi != ct.Esi)
		PrintReg(ct.Esi, F_H_RED);
	else
		PrintReg(ct.Esi, F_WHITE);

	if (oldCt.Edi != ct.Edi)
		PrintReg(ct.Edi, F_H_RED);
	else
		PrintReg(ct.Edi, F_WHITE);
	printf("\n");





	printf("    esp   |    ebp   ||||| CF | PF | AF | ZF | SF | TF | DF | OF |\n");
	PEFLAGS pEflgas = (PEFLAGS)&ct.EFlags;
	PEFLAGS pOldEflags = (PEFLAGS)&oldCt.EFlags;

	// 	printf(" %08X | %08X |||||%3d |%3d |%3d |%3d |%3d |%3d |%3d |%3d |\n",
	// 		   ct.Esp , ct.Ebp , pEflgas->CF , 
	// 		   pEflgas->PF , pEflgas->AF , pEflgas->ZF , 
	// 		   pEflgas->SF , pEflgas->TF , pEflgas->DF , pEflgas->OF
	// 		   );
	if (oldCt.Esp != ct.Esp)
		PrintReg(ct.Esp, F_H_RED);
	else
		PrintReg(ct.Esp, F_WHITE);
	if (oldCt.Ebp != ct.Ebp)
		PrintReg(ct.Ebp, F_H_RED);
	else
		PrintReg(ct.Ebp, F_WHITE);

	printf("||||");

	if (pOldEflags->CF != pEflgas->CF)
		PrintEflag(pEflgas->CF, F_H_RED);
	else
		PrintEflag(pEflgas->CF, F_WHITE);
	if (pOldEflags->PF != pEflgas->PF)
		PrintEflag(pEflgas->PF, F_H_RED);
	else
		PrintEflag(pEflgas->PF, F_WHITE);
	if (pOldEflags->AF != pEflgas->AF)
		PrintEflag(pEflgas->AF, F_H_RED);
	else
		PrintEflag(pEflgas->AF, F_WHITE);
	if (pOldEflags->ZF != pEflgas->ZF)
		PrintEflag(pEflgas->ZF, F_H_RED);
	else
		PrintEflag(pEflgas->ZF, F_WHITE);
	if (pOldEflags->SF != pEflgas->SF)
		PrintEflag(pEflgas->SF, F_H_RED);
	else
		PrintEflag(pEflgas->SF, F_WHITE);
	if (pOldEflags->TF != pEflgas->TF)
		PrintEflag(pEflgas->TF, F_H_RED);
	else
		PrintEflag(pEflgas->TF, F_WHITE);
	if (pOldEflags->DF != pEflgas->DF)
		PrintEflag(pEflgas->DF, F_H_RED);
	else
		PrintEflag(pEflgas->DF, F_WHITE);
	if (pOldEflags->OF != pEflgas->OF)
		PrintEflag(pEflgas->OF, F_H_RED);
	else
		PrintEflag(pEflgas->OF, F_WHITE);
	printf("\n");

	printf("--> eip  %08X\n", ct.Eip);
	oldCt = ct;
	printf("------------------------------------------------------------------\n");

}

void DbgUi::ShowMem(SIZE_T virtualAddress, const LPBYTE lpBuff, int nSize, DWORD dwShowFlag)
{
	printf("--------+--------------------------------------------------+------------------+\n");
	printf("        |  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  |                  |\n");
	printf("--------+--------------------------------------------------+------------------+\n");
	unsigned char ch = 0;
	const char *pAnsi = (char*)lpBuff;
	const wchar_t* pWchar = (wchar_t*)lpBuff;
	if (dwShowFlag)
	{
		printf("%08X| ", virtualAddress);
		virtualAddress += 16;
		unsigned short wch = 0;
		for (; nSize > 0; nSize -= 32)
		{
			for (int i = 0; i < 16; ++i)
			{
				wch = pWchar[i];
				if (dwShowFlag == 0)
					printf("%02X ", wch);
			}
			printf(" | ");
			for (int i = 0; i < 16; ++i)
			{
				if (ch < 33 || ch>126)
					ch = '.';
				wprintf(L"%c", wch);
			}
			printf(" |\n");

			pWchar += 16;
		}
	}
	else
	{
		for (; nSize > 0; nSize -= 16)
		{
			printf("%08X| ", virtualAddress);
			virtualAddress += 16;
			for (int i = 0; i < 16; ++i)
			{
				ch = pAnsi[i];
				printf("%02X ", ch);
			}
			printf(" | ");
			for (int i = 0; i < 16; ++i)
			{
				printf("%c", pAnsi[i] < 33 || pAnsi[i]>126 ? '.' : pAnsi[i]);
			}
			printf(" |\n");

			pAnsi += 16;
		}
	}
	printf("--------+--------------------------------------------------+------------------+\n");

}

void DbgUi::ShowStack(SIZE_T virtualAddress, const LPBYTE lpBuff, int nSize)
{
	SIZE_T	*p = (SIZE_T*)lpBuff;
	CString	buff;
	while (nSize >= 0)
	{
		printf(" %08X | %08X\n", virtualAddress, *p);
		++p;
		virtualAddress += sizeof(SIZE_T);
		nSize -= sizeof(SIZE_T);
	}
}

void DbgUi::ShowBreakPointList(list<BPObject*>::const_iterator beginItr, list<BPObject*>::const_iterator endItr)
{
	printf("------+------------+----------------+---------------\n");
	printf(" 序号 |    地址    |      类型      |     条件\n");
	printf("------+------------+----------------+---------------\n");
	int j = 0;


	for (; beginItr != endItr; ++beginItr)
	{
		E_BPType Type = (*beginItr)->Type();
		if (Type == breakpointType_tf)
		{
			++j;
			continue;
		}

		printf("%5d |", j++);
		printf(" 0x%08X |", (*beginItr)->GetAddress());
		const char* pCondition = nullptr;
		switch (Type)
		{
		case breakpointType_hard_r:
			printf("%14s  |", "硬件读断点");
			break;
		case breakpointType_hard_w:
			printf("%14s  |", "硬件写断点");
			break;
		case breakpointType_hard_e:
			printf("%14s  |", "硬件执行断点");
			break;
		case breakpointType_hard_rw:
			printf("%14s  |", "硬件读写断点");
			break;
		case breakpointType_acc_r:
			printf("%14s  |", "内存读断点");
			break;
		case breakpointType_acc_w:
			printf("%14s  |", "内存写断点");
			break;
		case breakpointType_acc_e:
			printf("%14s  |", "内存执行断点");
			break;
		case breakpointType_soft:
			printf("%14s  |", "软件断点");
			break;
		default:
			printf("%14s  |", "无类型");
		}
		pCondition = (*beginItr)->GetCondition();
		printf(" %s\n", pCondition == nullptr ? "无" : pCondition);
	}
	printf("------+------------+----------------+---------------\n");

}
