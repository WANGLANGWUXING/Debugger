#include "OutHex.h"
#include "DebuggerMain.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <conio.h>
using std::cout;
int value[MAX];

DWORD FileSize()
{
	//TCHAR szFileName[MAX_PATH] = TEXT("");
	HANDLE hFile = CreateFile(file_Name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		if (0 == GetLastError())
		{
			printf("file not exist");
		}
		return 0;
	}
	DWORD dwFileSize = 0;
	dwFileSize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return dwFileSize;
}

void HexToFile()
{
	ifstream fin(file_Name, ios::binary);

	ofstream in;
	in.open("exe.txt", ios::trunc);


	if (!fin)
		exit(0);
	char c;
	long i = 0, j = 0;
	//cout.setf(ios::uppercase);
	in.setf(ios::uppercase);
	//cout << setfill('0');
	in << setfill('0');
	DWORD leng = FileSize();
	while ((j * 16 + i) < leng)//while((c=fin.get())!=EOF)
	{
		c = fin.get();
		value[j * 16 + i] = (((int)c) & 0x000000ff);

		if (i == 0)
			//cout << hex << setw(7) << j << "0h: ";
			in << hex << setw(7) << j << "0h: ";
		//cout << hex << setw(2) << value[j * 16 + i] << " ";
		in << hex << setw(2) << value[j * 16 + i] << " ";
		if (i++ == 15)
		{
			//cout << endl;
			in << endl;
			i = 0;
			j++;
		}

	}
	fin.close();
	in.close();//关闭文件
}




void ExeTofile() {

	FILE *fpRead; //源
	FILE *fpWrite; // 目标
	int bufferLen = 1024 * 4; // 缓冲区长度
	char *buffer = (char*)malloc(bufferLen); // 开辟缓存
	int readCount; // 实际读取的字节数
	if (fopen_s(&fpRead, file_Name, "rb") != 0 ||
		fopen_s(&fpWrite, "exe.txt", "wb") != 0) {
		printf("Cannot open file, press any key to exit!\n");
		_getch();
		exit(1);
	}
	// 不断从fileRead读取内容，放在缓冲区，再将缓冲区的内容写入fileWrite
	while ((readCount = fread(buffer, 1, bufferLen, fpRead)) > 0) {
		fwrite(buffer, readCount, 1, fpWrite);
	}
	free(buffer);
	fclose(fpRead);
	fclose(fpWrite);

}



void dump(string str)
{
	//dump前还原所有断点
	//	for (auto temp : g_bps) {
	//		rmBreakpoint_cc(this->hProc, this->hThread, temp.address, temp.oldData);
	//	}

	HANDLE hFile = CreateFileA(str.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("创建文件失败,\n");
		if (GetLastError() == 0x00000050) {
			cout << "文件已存在！！！" << endl;
		}
		return;
	}
	IMAGE_DOS_HEADER dos;//dos头

	IMAGE_NT_HEADERS nt;
	//读dos头
	if (ReadProcessMemory(g_hProcess, (BYTE *)imgBase, &dos, sizeof(IMAGE_DOS_HEADER), NULL) == FALSE)
		return;


	//读nt头
	if (ReadProcessMemory(g_hProcess, (BYTE *)imgBase + dos.e_lfanew, &nt, sizeof(IMAGE_NT_HEADERS), NULL) == FALSE)
	{
		return;
	}


	//读取节区并计算节区大小
	DWORD secNum = nt.FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER Sections = new IMAGE_SECTION_HEADER[secNum];
	//读取节区
	if (ReadProcessMemory(g_hProcess,
		(BYTE *)imgBase + dos.e_lfanew + sizeof(IMAGE_NT_HEADERS),
		Sections,
		secNum * sizeof(IMAGE_SECTION_HEADER),
		NULL) == FALSE)
	{
		return;
	}

	//计算所有节区的大小
	DWORD allsecSize = 0;
	DWORD maxSec;//最大的节区

	maxSec = 0;

	for (int i = 0; i < secNum; ++i)
	{
		allsecSize += Sections[i].SizeOfRawData;

	}

	//dos
	//nt
	//节区总大小
	DWORD topsize = secNum * sizeof(IMAGE_SECTION_HEADER) + sizeof(IMAGE_NT_HEADERS) + dos.e_lfanew;

	//使头大小按照文件对齐
	if ((topsize&nt.OptionalHeader.FileAlignment) != topsize)
	{
		topsize &= nt.OptionalHeader.FileAlignment;
		topsize += nt.OptionalHeader.FileAlignment;
	}

	DWORD ftsize = topsize + allsecSize;
	//创建文件映射
	HANDLE hMap = CreateFileMapping(hFile,
		NULL, PAGE_READWRITE,
		0,
		ftsize,
		0);

	if (hMap == NULL)
	{
		printf("创建文件映射失败\n");
		return;
	}

	//创建视图
	LPVOID lpmem = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

	if (lpmem == NULL)
	{
		delete[] Sections;
		CloseHandle(hMap);
		printf("创建视图失败\n");
		return;
	}
	PBYTE bpMem = (PBYTE)lpmem;
	memcpy(lpmem, &dos, sizeof(IMAGE_DOS_HEADER));
	//计算dossub 大小

	DWORD subSize = dos.e_lfanew - sizeof(IMAGE_DOS_HEADER);

	if (ReadProcessMemory(g_hProcess, (BYTE *)imgBase + sizeof(IMAGE_DOS_HEADER), bpMem + sizeof(IMAGE_DOS_HEADER), subSize, NULL) == FALSE)
	{
		delete[] Sections;
		CloseHandle(hMap);
		UnmapViewOfFile(lpmem);
		return;
	}

	nt.OptionalHeader.ImageBase = (DWORD)imgBase;
	//保存NT头
	memcpy(bpMem + dos.e_lfanew, &nt, sizeof(IMAGE_NT_HEADERS));

	//保存节区
	memcpy(bpMem + dos.e_lfanew + sizeof(IMAGE_NT_HEADERS), Sections, secNum * sizeof(IMAGE_SECTION_HEADER));

	for (int i = 0; i < secNum; ++i)
	{
		if (ReadProcessMemory(
			g_hProcess, (BYTE *)imgBase + Sections[i].VirtualAddress,
			bpMem + Sections[i].PointerToRawData,
			Sections[i].SizeOfRawData,
			NULL) == FALSE)
		{
			delete[] Sections;
			CloseHandle(hMap);
			UnmapViewOfFile(lpmem);
			return;
		}
	}
	if (FlushViewOfFile(lpmem, 0) == false)
	{
		delete[] Sections;
		CloseHandle(hMap);
		UnmapViewOfFile(lpmem);
		printf("保存到文件失败\n");
		return;
	}
	delete[] Sections;
	CloseHandle(hMap);
	UnmapViewOfFile(lpmem);
	MessageBox(0, "ok", 0, 0);
	return;
}