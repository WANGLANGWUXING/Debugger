#include "Pectrl.h"
#include <stdio.h>
#include "atlstr.h"
#include "vector"
using namespace std;

vector<PIMAGE_IMPORT_DESCRIPTOR> m_VecImport;

/** 求一个值经过一定粒度对齐后的值 */
DWORD ToAligentSize(DWORD nSize, DWORD nAligent)
{
	// 超出了多少倍的内存对齐,超出多少倍,就有多少倍内存对齐单位 ;  
	// 零头是否超出内存对齐,超出则是一个内存对齐单位
	if (nSize%nAligent != 0)
		return (nSize / nAligent + 1)*nAligent;
	return nSize;
}

/** 虚拟内存偏移转文件偏移 */
DWORD RVAToOfs(const LPVOID pDosHdr, DWORD dwRVA)
{

	//开始遍历区段查找包含RVA地址的区段
	//获取标准头指针,以获取区段数目
	//获取区段数目
	DWORD	dwSecTotal = GetPEFileHdr(pDosHdr)->NumberOfSections;

	//获取第一个区段
	PIMAGE_SECTION_HEADER	pScn = GetPEFirScnHdr(pDosHdr);

	//遍历区段
	for (DWORD i = 0; i < dwSecTotal; i++)
	{
		if (dwRVA >= pScn->VirtualAddress
			&& dwRVA < pScn->VirtualAddress + pScn->Misc.VirtualSize)
		{
			// rva 转 文件偏移公式:
			// rva - 区段所在rva + 区段所在文件偏移
			return dwRVA - pScn->VirtualAddress + pScn->PointerToRawData;
		}
		++pScn;
	}
	return 0;
}

/** 获取PE的NT头 */
PIMAGE_NT_HEADERS GetPENtHdr(const LPVOID pDosHdr)
{
	return (PIMAGE_NT_HEADERS)((((PIMAGE_DOS_HEADER)(pDosHdr))->e_lfanew + (LPBYTE)(pDosHdr)));
}

/** 获取PE文件头 */
PIMAGE_FILE_HEADER GetPEFileHdr(const LPVOID pDosHdr)
{
	return &((PIMAGE_NT_HEADERS)((((PIMAGE_DOS_HEADER)(pDosHdr))->e_lfanew + (LPBYTE)(pDosHdr))))->FileHeader;
}

/** 获取PE扩展头 */
PIMAGE_OPTIONAL_HEADER32 GetPEOptHdr32(const LPVOID pDosHdr)
{
	return &((PIMAGE_NT_HEADERS)((((PIMAGE_DOS_HEADER)(pDosHdr))->e_lfanew + (LPBYTE)(pDosHdr))))->OptionalHeader;
}

/** 获取PE扩展头 */
PIMAGE_OPTIONAL_HEADER64 GetPEOptHdr64(const LPVOID pDosHdr)
{
	return &((PIMAGE_NT_HEADERS64)((((PIMAGE_DOS_HEADER)(pDosHdr))->e_lfanew + (LPBYTE)(pDosHdr))))->OptionalHeader;
}

/** 获取PE文件的第一个区段头 */
PIMAGE_SECTION_HEADER GetPEFirScnHdr(const LPVOID pDosHdr)
{
	return IMAGE_FIRST_SECTION(GetPENtHdr(pDosHdr));
}

PIMAGE_SECTION_HEADER GetPELastScnHdr(const LPVOID pDosHdr)
{
	DWORD	dwNumOfScn = GetPEFileHdr(pDosHdr)->NumberOfSections;
	return &GetPEFirScnHdr(pDosHdr)[dwNumOfScn - 1];
}


DWORD GetPEImpTab(const LPVOID pDosHdr, unsigned int *puSize /*= NULL*/)
{

	printf("序号\tDLL名称\t\t输入名称表\t日期时间标志\tForwarderChain\t名称\tFirstThunk\n");

	//获取导入表
	PIMAGE_IMPORT_DESCRIPTOR  pImpD;

	PIMAGE_DATA_DIRECTORY pDatD;//数据目录

	pDatD = GetPEDataDirTab(pDosHdr);

	pImpD = (PIMAGE_IMPORT_DESCRIPTOR)((long)buff + RVAToOfs(pDosHdr, pDatD[1].VirtualAddress));
	//第一层循环 每个导入的DLL依次解析
	int i = 0;
	CString str;
	while (pImpD->Name) {
		//DLL名称相关
		printf("%d\t", i + 1);
		printf("%s\t", (buff + RVAToOfs(pDosHdr, pImpD->Name)));

		printf("%08X\t", pImpD->OriginalFirstThunk);

		printf("%08X\t", pImpD->TimeDateStamp);

		printf("%08X\t", pImpD->ForwarderChain);

		printf("%08X\t", pImpD->Name);

		printf("%08X\n", pImpD->FirstThunk);

		m_VecImport.push_back(pImpD);
		i++;

		//system("pause");
		pImpD++;
	}



	return 0;
}


void ShowImpFunc(int pos)
{
	printf("ThunkRVA\tThunk偏移\tThunk值\t\t提示\tAPI名称或序号\n");

	int nIndex = pos;
	if (pos == NULL)//判断是否为空
	{
		return;
	}

	//从获取的DLL导入函数地址表 IAT 计算偏移

	PIMAGE_OPTIONAL_HEADER32 pOptH = GetPEOptHdr32(pDosH);


	if (pOptH != NULL) {
		PIMAGE_THUNK_DATA32 pInt = (PIMAGE_THUNK_DATA32)(buff + RVAToOfs(pDosH, m_VecImport[nIndex - 1]->FirstThunk));


		//循环解析导入地址表IAT
		int i = 0;
		CString str;
		while (pInt->u1.Function)
		{

			DWORD ThunkOffest = RVAToOfs(pDosH, m_VecImport[nIndex - 1]->OriginalFirstThunk);
			//判断最高位是否为1 不为1按名称导入
			if (!IMAGE_SNAP_BY_ORDINAL32(pInt->u1.Ordinal))
			{
				//找到函数序号名地址 并将其取出  
				printf("%08X\t", m_VecImport[nIndex - 1]->OriginalFirstThunk);

				printf("%08X\t", ThunkOffest);

				printf("%08X\t", pInt->u1.AddressOfData);

				PIMAGE_IMPORT_BY_NAME pFunName = (PIMAGE_IMPORT_BY_NAME)(buff + RVAToOfs(pDosH, pInt->u1.AddressOfData));

				printf("%04X\t", pFunName->Hint);

				printf("%s\n", pFunName->Name);

				//每次偏移四个字节
				m_VecImport[nIndex - 1]->OriginalFirstThunk += 4;
				ThunkOffest += 4;
			}
			else
			{

				////找到函数序号名地址 并将其取出  
				////	PIMAGE_IMPORT_BY_NAME pFunName=(PIMAGE_IMPORT_BY_NAME)(buf+CalcOffset(pInt->u1.AddressOfData,pNtH));
				printf("%08X\t", m_VecImport[nIndex - 1]->OriginalFirstThunk);

				printf("%08X\t", ThunkOffest);

				printf("%08X\t", pInt->u1.AddressOfData);

				printf("-\t");

				printf("%4xH  %4dD\n", pInt->u1.Ordinal & 0x7fffffff, pInt->u1.Ordinal & 0x7fffffff);

			}
			i++;
			pInt++;
		}
	}


	//PIMAGE_OPTIONAL_HEADER64 pOptH64 = GetPEOptHdr64(pDosH);


	//if (pOptH64 != NULL) {
	//	PIMAGE_THUNK_DATA64 pInt = (PIMAGE_THUNK_DATA64)(buff + RVAToOfs(pDosH, m_VecImport[nIndex - 1]->FirstThunk));

	//	//循环解析导入地址表IAT
	//	int i = 0;
	//	CString str;
	//	while (pInt->u1.Function)
	//	{

	//		DWORD ThunkOffest = RVAToOfs(pDosH, m_VecImport[nIndex - 1]->OriginalFirstThunk);
	//		//判断最高位是否为1 不为1按名称导入
	//		if (!IMAGE_SNAP_BY_ORDINAL32(pInt->u1.Ordinal))
	//		{
	//			//找到函数序号名地址 并将其取出  
	//			printf("%08X   ", m_VecImport[nIndex - 1]->OriginalFirstThunk);

	//			printf("%08X   ", ThunkOffest);

	//			printf("%08X   ", pInt->u1.AddressOfData);

	//			PIMAGE_IMPORT_BY_NAME pFunName = (PIMAGE_IMPORT_BY_NAME)(buff + RVAToOfs(pDosH, pInt->u1.AddressOfData));

	//			printf("%04X   ", pFunName->Hint);

	//			printf("%s\n", pFunName->Name);

	//			//每次偏移四个字节
	//			m_VecImport[nIndex - 1]->OriginalFirstThunk += 4;
	//			ThunkOffest += 4;
	//		}
	//		else
	//		{

	//			////找到函数序号名地址 并将其取出  
	//			////	PIMAGE_IMPORT_BY_NAME pFunName=(PIMAGE_IMPORT_BY_NAME)(buf+CalcOffset(pInt->u1.AddressOfData,pNtH));
	//			printf("%08X   ", m_VecImport[nIndex - 1]->OriginalFirstThunk);

	//			printf("%08X   ", ThunkOffest);

	//			printf("%08X   ", pInt->u1.AddressOfData);

	//			printf("-   ");

	//			printf("%4xH  %4dD\n", pInt->u1.Ordinal & 0x7fffffff, pInt->u1.Ordinal & 0x7fffffff);

	//		}
	//		i++;
	//		pInt++;
	//	}
	//}


}

/** 判断是否是一个有效的PE文件 */
bool isPeFile(const LPVOID pDosHdr)
{
	return (*((WORD*)pDosHdr) == 'ZM') && (*((WORD*)GetPENtHdr(pDosHdr)) == 'EP');
}

/** 获取PE文件的映像大小 */
DWORD GetPEImageSize(const LPVOID pDosHdr)
{
	DWORD dwSize = 0;
	PIMAGE_OPTIONAL_HEADER32 pOptHdr = NULL;
	pOptHdr = GetPEOptHdr32(pDosHdr);

	if (pOptHdr->Magic == 0x10B)
	{
		dwSize = pOptHdr->SizeOfImage;
	}
	else
	{
		PIMAGE_OPTIONAL_HEADER64 pOptHdr64 = NULL;
		pOptHdr64 = GetPEOptHdr64(pDosHdr);
		dwSize = pOptHdr64->SizeOfImage;
	}

	return dwSize;
}


/** 获取PE文件的整个头部的大小(包括DOS,PE头,区段表头) */
DWORD GetPEHdrSize(const LPVOID pDosHdr)
{
	if (pDosHdr == NULL)
		return 0;

	// 获取PE文件头部大小
	PIMAGE_OPTIONAL_HEADER32 pOptHdr = NULL;/*option header 扩展头*/
	pOptHdr = GetPEOptHdr32(pDosHdr);
	DWORD dwHdrSize = 0;
	if (pOptHdr->Magic == 0x10B) //32位的程序
	{
		dwHdrSize = pOptHdr->SizeOfHeaders;
	}
	else
	{
		PIMAGE_OPTIONAL_HEADER64 pOptHdr64 = NULL;
		pOptHdr64 = GetPEOptHdr64(pDosHdr);
		dwHdrSize = pOptHdr64->SizeOfHeaders;
	}
	return dwHdrSize;

}

/** 获取映像大小 */
DWORD GetPEImgSize(const LPVOID pDosHdr)
{
	if (pDosHdr == NULL)
		return 0;

	PIMAGE_OPTIONAL_HEADER32 pOptHdr = NULL;
	pOptHdr = GetPEOptHdr32(pDosHdr);

	if (pOptHdr->Magic == 0x10B)
	{
		return pOptHdr->SizeOfImage;
	}
	else
	{
		PIMAGE_OPTIONAL_HEADER64 pOptHdr64 = NULL;
		pOptHdr64 = GetPEOptHdr64(pDosHdr);
		return pOptHdr64->SizeOfImage;
	}
	return 0;
}

/** 设置PE文件整个头部的大小(改变扩展头中的SizeOfImage字段 */
void SetPEImgSize(const LPVOID pDosHdr, DWORD dwSize)
{
	if (pDosHdr == NULL)
		return;

	PIMAGE_OPTIONAL_HEADER32 pOptHdr = NULL;
	pOptHdr = GetPEOptHdr32(pDosHdr);

	if (pOptHdr->Magic == 0x10B)
	{
		pOptHdr->SizeOfImage = dwSize;
	}
	else
	{
		PIMAGE_OPTIONAL_HEADER64 pOptHdr64 = NULL;
		pOptHdr64 = GetPEOptHdr64(pDosHdr);
		pOptHdr64->SizeOfImage = dwSize;
	}

}


/** 获取数据目录表 */
PIMAGE_DATA_DIRECTORY GetPEDataDirTab(const LPVOID pDosHdr)
{
	// 得到导入表的文件偏移
	PIMAGE_OPTIONAL_HEADER32 pOptHdr = NULL;
	pOptHdr = GetPEOptHdr32(pDosHdr);

	PIMAGE_DATA_DIRECTORY pDataDir = NULL;

	if (pOptHdr->Magic == 0x10B)
	{
		pDataDir = pOptHdr->DataDirectory;
	}
	else
	{
		PIMAGE_OPTIONAL_HEADER64 pOptHdr64 = NULL;
		pOptHdr64 = GetPEOptHdr64(pDosHdr);
		pDataDir = pOptHdr64->DataDirectory;
	}
	return pDataDir;
}


PDWORD pFunAddr;
PDWORD pFunNameAddr;
PWORD pOrdinalAddr;
DWORD NumberOfFun;
DWORD NumberOfName;

PIMAGE_EXPORT_DIRECTORY pExpD;

DWORD GetPEExpTab(const LPVOID pDosHdr, unsigned int *puSize /*= NULL*/)
{
	//获取数据目录

	PIMAGE_DATA_DIRECTORY pDatD = GetPEDataDirTab(pDosHdr);

	//获取导出表数据
	pExpD = NULL;

	pExpD = (PIMAGE_EXPORT_DIRECTORY)(buff + RVAToOfs(pDosH, pDatD[0].VirtualAddress));
	//判断是否有导出函数

	printf("输出表地址：\t%08X  \n", pExpD);

	//取出三个表的函数地址
	pFunAddr = (PDWORD)(buff + RVAToOfs(pDosH, pExpD->AddressOfFunctions));	//函数地址
	printf("函数地址：\t%08X\n", pExpD->AddressOfFunctions);

	pFunNameAddr = (PDWORD)(buff + RVAToOfs(pDosH, pExpD->AddressOfNames));	//函数名地址
	printf("函数名地址：\t%08X\n", pExpD->AddressOfNames);

	pOrdinalAddr = (PWORD)(buff + RVAToOfs(pDosH, pExpD->AddressOfNameOrdinals));//函数序号地址
	printf("函数序号地址：\t%08X\n", pExpD->AddressOfNameOrdinals);


	printf("基址：\t\t%08X\n",pExpD->Base);


	printf("特征值：\t%08X\n", pExpD->Characteristics);

	printf("名称：\t\t%08X\n", pExpD->Name);
	printf("名称字符串:\t%s\n", RVAToOfs(pDosH, pExpD->Name)+buff);
	printf("函数数量：\t%08X\n", pExpD->NumberOfFunctions);
	printf("函数名称数量：\t%08X\n", pExpD->NumberOfNames);
	NumberOfFun = pExpD->NumberOfFunctions;
	NumberOfName = pExpD->NumberOfNames;
	return 0;
}


void ShowExpFunc()
{
	printf("序号\tRVA\t\t偏移\t\t函数\n");

	for (DWORD i = 0; i < NumberOfFun; i++)
	{
		//如果是无效函数 进行下一次
		if (!pFunAddr[i])
		{
			continue;
		}
		//此时为有效函数  在序号表查找是否有这个序号 用以判断是函数名导出,函数序号导出
		DWORD j = 0;
		CString str;
		for (; j < NumberOfName; j++)
		{

			if (i == pOrdinalAddr[j])
			{
				break;
			}
		}

		//函数名导出的函数
		if (j != NumberOfName)
		{
			printf("%d\t", pOrdinalAddr[j] + pExpD->Base);

			printf("%08X\t", pFunAddr[i]);


			printf("%08X\t", RVAToOfs(pDosH, pFunAddr[i]));

			printf("%s\n", (buff + RVAToOfs(pDosH, pFunNameAddr[j])));

		}
		//序号导出的函数 没有名字
		else
		{
			printf("%d", i + pExpD->Base);

			printf("-      -      -\n");

		}
	}



}

// 获取模块的函数 返回函数的VA
LPVOID GetPEProcAddress(LPVOID hModule, LPVOID pExpTab, const char* pszName)
{
	DWORD	uNumOfName;/*函数名称个数*/
	DWORD	uNumOfFun;/*函数总个数*/
	PSIZE_T	puFunAddr;/*函数地址表*/
	PSIZE_T	puFunName;/*函数名称表*/
	PWORD	pwFunOrd;/*函数序号表*/
	PIMAGE_EXPORT_DIRECTORY pExp = (PIMAGE_EXPORT_DIRECTORY)pExpTab;
	if (pExpTab == 0)
		return 0;

	/*获取名称表*/
	puFunName = (PSIZE_T)(RVAToOfs(hModule, pExp->AddressOfNames) + (DWORD)hModule);
	/*获取地址表*/
	puFunAddr = (PSIZE_T)(RVAToOfs(hModule, pExp->AddressOfFunctions) + (DWORD)hModule);
	/*获取序号表*/
	pwFunOrd = (PWORD)(RVAToOfs(hModule, pExp->AddressOfNameOrdinals) + (DWORD)hModule);

	// 获取名称数量
	uNumOfName = pExp->NumberOfNames;
	uNumOfFun = pExp->NumberOfFunctions;

	char* pName = 0;;
	// 遍历序号表
	for (DWORD i = 0; i < uNumOfFun; ++i)
	{
		DWORD j = 0;
		for (; j < uNumOfName; ++j)
		{
			// 含有名称
			if (i == pwFunOrd[j])
				break;
		}
		if (j < uNumOfFun)
		{
			pName = (char*)(RVAToOfs(hModule, puFunName[j]) + (DWORD)hModule);
			if (strcmp(pName, pszName) == 0)
			{
				// 返回一个VA
				return (LPVOID)(puFunAddr[i] + (DWORD)hModule);
			}
		}
	}
	return 0;
}


void AnalysisPE(char* FileName)
{
	if (buff != NULL) {
		delete buff;
		buff = NULL;
	}

	pDosH = NULL;

	buff = nullptr;
	//得到文件句柄
	HANDLE hFile = CreateFile(
		FileName, GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//MessageBox(0, "文件打开失败\n", 0, 0);
		return;
	}

	//得到文件大小
	DWORD dwFileSize = GetFileSize(hFile, NULL);

	DWORD ReadSize = 0;

	buff = new char[dwFileSize];
	//将文件读取到内存
	ReadFile(hFile, buff, dwFileSize, &
		ReadSize, NULL);
	//获取dos头
	pDosH = (PIMAGE_DOS_HEADER)buff;



	if (*(PWORD)buff != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(NULL, "不是有效的PE文件", "错误", MB_OK);
		return;
	}
	pDosH = (IMAGE_DOS_HEADER*)buff;
	if (*(PDWORD)((byte*)buff + pDosH->e_lfanew) != IMAGE_NT_SIGNATURE)
	{
		MessageBox(NULL, "不是有效的PE文件", "错误", MB_OK);
		return;
	}

	CloseHandle(hFile);

}

void FixPEImgSize(const LPVOID pDosHdr)
{
	PIMAGE_SECTION_HEADER pLastScn = GetPELastScnHdr(pDosHdr);
	if (pLastScn == NULL)
		return;
	SetPEImgSize(pDosHdr, pLastScn->VirtualAddress + pLastScn->SizeOfRawData);
}

PIMAGE_SECTION_HEADER GetPESection(const LPVOID pDosHdr, DWORD dwRVA)
{
	DWORD	dwNumOfScn = GetPEFileHdr(pDosHdr)->NumberOfSections;
	PIMAGE_SECTION_HEADER pScn = GetPEFirScnHdr(pDosHdr);
	for (DWORD i = 0; i < dwNumOfScn; i++, ++pScn)
	{
		if (pScn->VirtualAddress <= dwRVA && dwRVA <= pScn->VirtualAddress + pScn->SizeOfRawData)
			return pScn;
	}
	return NULL;
}