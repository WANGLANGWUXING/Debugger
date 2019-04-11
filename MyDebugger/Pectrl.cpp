#include "Pectrl.h"
#include <stdio.h>
#include "atlstr.h"
#include "vector"
using namespace std;

vector<PIMAGE_IMPORT_DESCRIPTOR> m_VecImport;

/** ��һ��ֵ����һ�����ȶ�����ֵ */
DWORD ToAligentSize(DWORD nSize, DWORD nAligent)
{
	// �����˶��ٱ����ڴ����,�������ٱ�,���ж��ٱ��ڴ���뵥λ ;  
	// ��ͷ�Ƿ񳬳��ڴ����,��������һ���ڴ���뵥λ
	if (nSize%nAligent != 0)
		return (nSize / nAligent + 1)*nAligent;
	return nSize;
}

/** �����ڴ�ƫ��ת�ļ�ƫ�� */
DWORD RVAToOfs(const LPVOID pDosHdr, DWORD dwRVA)
{

	//��ʼ�������β��Ұ���RVA��ַ������
	//��ȡ��׼ͷָ��,�Ի�ȡ������Ŀ
	//��ȡ������Ŀ
	DWORD	dwSecTotal = GetPEFileHdr(pDosHdr)->NumberOfSections;

	//��ȡ��һ������
	PIMAGE_SECTION_HEADER	pScn = GetPEFirScnHdr(pDosHdr);

	//��������
	for (DWORD i = 0; i < dwSecTotal; i++)
	{
		if (dwRVA >= pScn->VirtualAddress
			&& dwRVA < pScn->VirtualAddress + pScn->Misc.VirtualSize)
		{
			// rva ת �ļ�ƫ�ƹ�ʽ:
			// rva - ��������rva + ���������ļ�ƫ��
			return dwRVA - pScn->VirtualAddress + pScn->PointerToRawData;
		}
		++pScn;
	}
	return 0;
}

/** ��ȡPE��NTͷ */
PIMAGE_NT_HEADERS GetPENtHdr(const LPVOID pDosHdr)
{
	return (PIMAGE_NT_HEADERS)((((PIMAGE_DOS_HEADER)(pDosHdr))->e_lfanew + (LPBYTE)(pDosHdr)));
}

/** ��ȡPE�ļ�ͷ */
PIMAGE_FILE_HEADER GetPEFileHdr(const LPVOID pDosHdr)
{
	return &((PIMAGE_NT_HEADERS)((((PIMAGE_DOS_HEADER)(pDosHdr))->e_lfanew + (LPBYTE)(pDosHdr))))->FileHeader;
}

/** ��ȡPE��չͷ */
PIMAGE_OPTIONAL_HEADER32 GetPEOptHdr32(const LPVOID pDosHdr)
{
	return &((PIMAGE_NT_HEADERS)((((PIMAGE_DOS_HEADER)(pDosHdr))->e_lfanew + (LPBYTE)(pDosHdr))))->OptionalHeader;
}

/** ��ȡPE��չͷ */
PIMAGE_OPTIONAL_HEADER64 GetPEOptHdr64(const LPVOID pDosHdr)
{
	return &((PIMAGE_NT_HEADERS64)((((PIMAGE_DOS_HEADER)(pDosHdr))->e_lfanew + (LPBYTE)(pDosHdr))))->OptionalHeader;
}

/** ��ȡPE�ļ��ĵ�һ������ͷ */
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

	printf("���\tDLL����\t\t�������Ʊ�\t����ʱ���־\tForwarderChain\t����\tFirstThunk\n");

	//��ȡ�����
	PIMAGE_IMPORT_DESCRIPTOR  pImpD;

	PIMAGE_DATA_DIRECTORY pDatD;//����Ŀ¼

	pDatD = GetPEDataDirTab(pDosHdr);

	pImpD = (PIMAGE_IMPORT_DESCRIPTOR)((long)buff + RVAToOfs(pDosHdr, pDatD[1].VirtualAddress));
	//��һ��ѭ�� ÿ�������DLL���ν���
	int i = 0;
	CString str;
	while (pImpD->Name) {
		//DLL�������
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
	printf("ThunkRVA\tThunkƫ��\tThunkֵ\t\t��ʾ\tAPI���ƻ����\n");

	int nIndex = pos;
	if (pos == NULL)//�ж��Ƿ�Ϊ��
	{
		return;
	}

	//�ӻ�ȡ��DLL���뺯����ַ�� IAT ����ƫ��

	PIMAGE_OPTIONAL_HEADER32 pOptH = GetPEOptHdr32(pDosH);


	if (pOptH != NULL) {
		PIMAGE_THUNK_DATA32 pInt = (PIMAGE_THUNK_DATA32)(buff + RVAToOfs(pDosH, m_VecImport[nIndex - 1]->FirstThunk));


		//ѭ�����������ַ��IAT
		int i = 0;
		CString str;
		while (pInt->u1.Function)
		{

			DWORD ThunkOffest = RVAToOfs(pDosH, m_VecImport[nIndex - 1]->OriginalFirstThunk);
			//�ж����λ�Ƿ�Ϊ1 ��Ϊ1�����Ƶ���
			if (!IMAGE_SNAP_BY_ORDINAL32(pInt->u1.Ordinal))
			{
				//�ҵ������������ַ ������ȡ��  
				printf("%08X\t", m_VecImport[nIndex - 1]->OriginalFirstThunk);

				printf("%08X\t", ThunkOffest);

				printf("%08X\t", pInt->u1.AddressOfData);

				PIMAGE_IMPORT_BY_NAME pFunName = (PIMAGE_IMPORT_BY_NAME)(buff + RVAToOfs(pDosH, pInt->u1.AddressOfData));

				printf("%04X\t", pFunName->Hint);

				printf("%s\n", pFunName->Name);

				//ÿ��ƫ���ĸ��ֽ�
				m_VecImport[nIndex - 1]->OriginalFirstThunk += 4;
				ThunkOffest += 4;
			}
			else
			{

				////�ҵ������������ַ ������ȡ��  
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

	//	//ѭ�����������ַ��IAT
	//	int i = 0;
	//	CString str;
	//	while (pInt->u1.Function)
	//	{

	//		DWORD ThunkOffest = RVAToOfs(pDosH, m_VecImport[nIndex - 1]->OriginalFirstThunk);
	//		//�ж����λ�Ƿ�Ϊ1 ��Ϊ1�����Ƶ���
	//		if (!IMAGE_SNAP_BY_ORDINAL32(pInt->u1.Ordinal))
	//		{
	//			//�ҵ������������ַ ������ȡ��  
	//			printf("%08X   ", m_VecImport[nIndex - 1]->OriginalFirstThunk);

	//			printf("%08X   ", ThunkOffest);

	//			printf("%08X   ", pInt->u1.AddressOfData);

	//			PIMAGE_IMPORT_BY_NAME pFunName = (PIMAGE_IMPORT_BY_NAME)(buff + RVAToOfs(pDosH, pInt->u1.AddressOfData));

	//			printf("%04X   ", pFunName->Hint);

	//			printf("%s\n", pFunName->Name);

	//			//ÿ��ƫ���ĸ��ֽ�
	//			m_VecImport[nIndex - 1]->OriginalFirstThunk += 4;
	//			ThunkOffest += 4;
	//		}
	//		else
	//		{

	//			////�ҵ������������ַ ������ȡ��  
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

/** �ж��Ƿ���һ����Ч��PE�ļ� */
bool isPeFile(const LPVOID pDosHdr)
{
	return (*((WORD*)pDosHdr) == 'ZM') && (*((WORD*)GetPENtHdr(pDosHdr)) == 'EP');
}

/** ��ȡPE�ļ���ӳ���С */
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


/** ��ȡPE�ļ�������ͷ���Ĵ�С(����DOS,PEͷ,���α�ͷ) */
DWORD GetPEHdrSize(const LPVOID pDosHdr)
{
	if (pDosHdr == NULL)
		return 0;

	// ��ȡPE�ļ�ͷ����С
	PIMAGE_OPTIONAL_HEADER32 pOptHdr = NULL;/*option header ��չͷ*/
	pOptHdr = GetPEOptHdr32(pDosHdr);
	DWORD dwHdrSize = 0;
	if (pOptHdr->Magic == 0x10B) //32λ�ĳ���
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

/** ��ȡӳ���С */
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

/** ����PE�ļ�����ͷ���Ĵ�С(�ı���չͷ�е�SizeOfImage�ֶ� */
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


/** ��ȡ����Ŀ¼�� */
PIMAGE_DATA_DIRECTORY GetPEDataDirTab(const LPVOID pDosHdr)
{
	// �õ��������ļ�ƫ��
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
	//��ȡ����Ŀ¼

	PIMAGE_DATA_DIRECTORY pDatD = GetPEDataDirTab(pDosHdr);

	//��ȡ����������
	pExpD = NULL;

	pExpD = (PIMAGE_EXPORT_DIRECTORY)(buff + RVAToOfs(pDosH, pDatD[0].VirtualAddress));
	//�ж��Ƿ��е�������

	printf("������ַ��\t%08X  \n", pExpD);

	//ȡ��������ĺ�����ַ
	pFunAddr = (PDWORD)(buff + RVAToOfs(pDosH, pExpD->AddressOfFunctions));	//������ַ
	printf("������ַ��\t%08X\n", pExpD->AddressOfFunctions);

	pFunNameAddr = (PDWORD)(buff + RVAToOfs(pDosH, pExpD->AddressOfNames));	//��������ַ
	printf("��������ַ��\t%08X\n", pExpD->AddressOfNames);

	pOrdinalAddr = (PWORD)(buff + RVAToOfs(pDosH, pExpD->AddressOfNameOrdinals));//������ŵ�ַ
	printf("������ŵ�ַ��\t%08X\n", pExpD->AddressOfNameOrdinals);


	printf("��ַ��\t\t%08X\n",pExpD->Base);


	printf("����ֵ��\t%08X\n", pExpD->Characteristics);

	printf("���ƣ�\t\t%08X\n", pExpD->Name);
	printf("�����ַ���:\t%s\n", RVAToOfs(pDosH, pExpD->Name)+buff);
	printf("����������\t%08X\n", pExpD->NumberOfFunctions);
	printf("��������������\t%08X\n", pExpD->NumberOfNames);
	NumberOfFun = pExpD->NumberOfFunctions;
	NumberOfName = pExpD->NumberOfNames;
	return 0;
}


void ShowExpFunc()
{
	printf("���\tRVA\t\tƫ��\t\t����\n");

	for (DWORD i = 0; i < NumberOfFun; i++)
	{
		//�������Ч���� ������һ��
		if (!pFunAddr[i])
		{
			continue;
		}
		//��ʱΪ��Ч����  ����ű�����Ƿ��������� �����ж��Ǻ���������,������ŵ���
		DWORD j = 0;
		CString str;
		for (; j < NumberOfName; j++)
		{

			if (i == pOrdinalAddr[j])
			{
				break;
			}
		}

		//�����������ĺ���
		if (j != NumberOfName)
		{
			printf("%d\t", pOrdinalAddr[j] + pExpD->Base);

			printf("%08X\t", pFunAddr[i]);


			printf("%08X\t", RVAToOfs(pDosH, pFunAddr[i]));

			printf("%s\n", (buff + RVAToOfs(pDosH, pFunNameAddr[j])));

		}
		//��ŵ����ĺ��� û������
		else
		{
			printf("%d", i + pExpD->Base);

			printf("-      -      -\n");

		}
	}



}

// ��ȡģ��ĺ��� ���غ�����VA
LPVOID GetPEProcAddress(LPVOID hModule, LPVOID pExpTab, const char* pszName)
{
	DWORD	uNumOfName;/*�������Ƹ���*/
	DWORD	uNumOfFun;/*�����ܸ���*/
	PSIZE_T	puFunAddr;/*������ַ��*/
	PSIZE_T	puFunName;/*�������Ʊ�*/
	PWORD	pwFunOrd;/*������ű�*/
	PIMAGE_EXPORT_DIRECTORY pExp = (PIMAGE_EXPORT_DIRECTORY)pExpTab;
	if (pExpTab == 0)
		return 0;

	/*��ȡ���Ʊ�*/
	puFunName = (PSIZE_T)(RVAToOfs(hModule, pExp->AddressOfNames) + (DWORD)hModule);
	/*��ȡ��ַ��*/
	puFunAddr = (PSIZE_T)(RVAToOfs(hModule, pExp->AddressOfFunctions) + (DWORD)hModule);
	/*��ȡ��ű�*/
	pwFunOrd = (PWORD)(RVAToOfs(hModule, pExp->AddressOfNameOrdinals) + (DWORD)hModule);

	// ��ȡ��������
	uNumOfName = pExp->NumberOfNames;
	uNumOfFun = pExp->NumberOfFunctions;

	char* pName = 0;;
	// ������ű�
	for (DWORD i = 0; i < uNumOfFun; ++i)
	{
		DWORD j = 0;
		for (; j < uNumOfName; ++j)
		{
			// ��������
			if (i == pwFunOrd[j])
				break;
		}
		if (j < uNumOfFun)
		{
			pName = (char*)(RVAToOfs(hModule, puFunName[j]) + (DWORD)hModule);
			if (strcmp(pName, pszName) == 0)
			{
				// ����һ��VA
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
	//�õ��ļ����
	HANDLE hFile = CreateFile(
		FileName, GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//MessageBox(0, "�ļ���ʧ��\n", 0, 0);
		return;
	}

	//�õ��ļ���С
	DWORD dwFileSize = GetFileSize(hFile, NULL);

	DWORD ReadSize = 0;

	buff = new char[dwFileSize];
	//���ļ���ȡ���ڴ�
	ReadFile(hFile, buff, dwFileSize, &
		ReadSize, NULL);
	//��ȡdosͷ
	pDosH = (PIMAGE_DOS_HEADER)buff;



	if (*(PWORD)buff != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(NULL, "������Ч��PE�ļ�", "����", MB_OK);
		return;
	}
	pDosH = (IMAGE_DOS_HEADER*)buff;
	if (*(PDWORD)((byte*)buff + pDosH->e_lfanew) != IMAGE_NT_SIGNATURE)
	{
		MessageBox(NULL, "������Ч��PE�ļ�", "����", MB_OK);
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