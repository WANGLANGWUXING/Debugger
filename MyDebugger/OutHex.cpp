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
	in.close();//�ر��ļ�
}




void ExeTofile() {

	FILE *fpRead; //Դ
	FILE *fpWrite; // Ŀ��
	int bufferLen = 1024 * 4; // ����������
	char *buffer = (char*)malloc(bufferLen); // ���ٻ���
	int readCount; // ʵ�ʶ�ȡ���ֽ���
	if (fopen_s(&fpRead, file_Name, "rb") != 0 ||
		fopen_s(&fpWrite, "exe.txt", "wb") != 0) {
		printf("Cannot open file, press any key to exit!\n");
		_getch();
		exit(1);
	}
	// ���ϴ�fileRead��ȡ���ݣ����ڻ��������ٽ�������������д��fileWrite
	while ((readCount = fread(buffer, 1, bufferLen, fpRead)) > 0) {
		fwrite(buffer, readCount, 1, fpWrite);
	}
	free(buffer);
	fclose(fpRead);
	fclose(fpWrite);

}



void dump(string str)
{
	//dumpǰ��ԭ���жϵ�
	//	for (auto temp : g_bps) {
	//		rmBreakpoint_cc(this->hProc, this->hThread, temp.address, temp.oldData);
	//	}

	HANDLE hFile = CreateFileA(str.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("�����ļ�ʧ��,\n");
		if (GetLastError() == 0x00000050) {
			cout << "�ļ��Ѵ��ڣ�����" << endl;
		}
		return;
	}
	IMAGE_DOS_HEADER dos;//dosͷ

	IMAGE_NT_HEADERS nt;
	//��dosͷ
	if (ReadProcessMemory(g_hProcess, (BYTE *)imgBase, &dos, sizeof(IMAGE_DOS_HEADER), NULL) == FALSE)
		return;


	//��ntͷ
	if (ReadProcessMemory(g_hProcess, (BYTE *)imgBase + dos.e_lfanew, &nt, sizeof(IMAGE_NT_HEADERS), NULL) == FALSE)
	{
		return;
	}


	//��ȡ���������������С
	DWORD secNum = nt.FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER Sections = new IMAGE_SECTION_HEADER[secNum];
	//��ȡ����
	if (ReadProcessMemory(g_hProcess,
		(BYTE *)imgBase + dos.e_lfanew + sizeof(IMAGE_NT_HEADERS),
		Sections,
		secNum * sizeof(IMAGE_SECTION_HEADER),
		NULL) == FALSE)
	{
		return;
	}

	//�������н����Ĵ�С
	DWORD allsecSize = 0;
	DWORD maxSec;//���Ľ���

	maxSec = 0;

	for (int i = 0; i < secNum; ++i)
	{
		allsecSize += Sections[i].SizeOfRawData;

	}

	//dos
	//nt
	//�����ܴ�С
	DWORD topsize = secNum * sizeof(IMAGE_SECTION_HEADER) + sizeof(IMAGE_NT_HEADERS) + dos.e_lfanew;

	//ʹͷ��С�����ļ�����
	if ((topsize&nt.OptionalHeader.FileAlignment) != topsize)
	{
		topsize &= nt.OptionalHeader.FileAlignment;
		topsize += nt.OptionalHeader.FileAlignment;
	}

	DWORD ftsize = topsize + allsecSize;
	//�����ļ�ӳ��
	HANDLE hMap = CreateFileMapping(hFile,
		NULL, PAGE_READWRITE,
		0,
		ftsize,
		0);

	if (hMap == NULL)
	{
		printf("�����ļ�ӳ��ʧ��\n");
		return;
	}

	//������ͼ
	LPVOID lpmem = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

	if (lpmem == NULL)
	{
		delete[] Sections;
		CloseHandle(hMap);
		printf("������ͼʧ��\n");
		return;
	}
	PBYTE bpMem = (PBYTE)lpmem;
	memcpy(lpmem, &dos, sizeof(IMAGE_DOS_HEADER));
	//����dossub ��С

	DWORD subSize = dos.e_lfanew - sizeof(IMAGE_DOS_HEADER);

	if (ReadProcessMemory(g_hProcess, (BYTE *)imgBase + sizeof(IMAGE_DOS_HEADER), bpMem + sizeof(IMAGE_DOS_HEADER), subSize, NULL) == FALSE)
	{
		delete[] Sections;
		CloseHandle(hMap);
		UnmapViewOfFile(lpmem);
		return;
	}

	nt.OptionalHeader.ImageBase = (DWORD)imgBase;
	//����NTͷ
	memcpy(bpMem + dos.e_lfanew, &nt, sizeof(IMAGE_NT_HEADERS));

	//�������
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
		printf("���浽�ļ�ʧ��\n");
		return;
	}
	delete[] Sections;
	CloseHandle(hMap);
	UnmapViewOfFile(lpmem);
	MessageBox(0, "ok", 0, 0);
	return;
}