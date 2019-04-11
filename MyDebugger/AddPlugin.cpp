#include "AddPlugin.h"
#include <windows.h>
#include <vector>
using std::vector;
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
//1. exeĿ¼���½�һ��plugin�ļ���
//2. ���ز������ - ��ʾ�����Ϣ
//3. ����plugin������dll�ļ�
//4. ��̬����dll
//5. ��ȡ��������Ϣ
//6. ָ��λ�õ��ò���ڲ�����


//dll ��������
//bool InitPlugin(char plugin_name[250], DWORD *plugin_version);
//dll �������ܺ���
//bool DrawShape();

typedef bool(*InitPlugin)(char *, DWORD *);
typedef bool(*DrawShape)();



//�����Ϣ
typedef struct _MYPLUGIN {
	char plugin_name[250];
	DWORD plugin_vesrion;
	HMODULE plugin_hmodule;
	DrawShape pfunc;
}MYPLUGIN, *PMYPLUGIN;

//�������
vector<MYPLUGIN> g_vecPlugin;

void LoadPlugin()
{
	printf("���������......\n");
	//����plugin������dll
	WIN32_FIND_DATAA wfd = {};
	HANDLE hFile = FindFirstFileA(".\\plugin\\*.dll", &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("�������ʧ��\n");
		return;
	}
	do
	{
		MYPLUGIN myplugin;
		//ƴ��dll·��
		char dllname[250];
		sprintf_s(dllname, ".\\plugin\\%s", wfd.cFileName);
		//��ȡ��׺��
		if (strcmp(PathFindExtensionA(wfd.cFileName), ".dll") != 0)
		{
			continue;
		}

		//����dll
		HMODULE hModule = LoadLibraryA(dllname);
		if (hModule == NULL)
		{
			continue;
		}

		//��ȡ�����ĺ���
		InitPlugin pFunc = (InitPlugin)GetProcAddress(hModule, "InitPlugin");
		//��ȡ��� ���֣�����汾
		pFunc(myplugin.plugin_name, &myplugin.plugin_vesrion);
		myplugin.plugin_hmodule = hModule;
		//��ȡ������ܺ���
		myplugin.pfunc = (DrawShape)GetProcAddress(hModule, "DrawShape");
		g_vecPlugin.push_back(myplugin);

	} while (FindNextFileA(hFile, &wfd));

	printf("������سɹ�\n");
}

void PluginInit()
{
	LoadPlugin();
	if (g_vecPlugin.size() > 0)
	{
		printf("���\t�����\n");
		//��ʾ�������
		for (int i = 0; i < g_vecPlugin.size(); i++)
		{
			printf("%d\t%s\n", i + 1, g_vecPlugin[i].plugin_name);
		}

		//���ò������
		DWORD dwSel;
		char cmdLine[10];
		do
		{
			printf("����������ţ�");
			scanf_s("%d", &dwSel);
			dwSel = dwSel - 1;
			if (dwSel < g_vecPlugin.size() && dwSel >= 0)
			{
				g_vecPlugin[dwSel].pfunc();
			}
			else {
				printf("��Ŵ�������������.\n");
				continue;
			}
			printf("�Ƿ������(Y/N)");
			scanf_s("%s", cmdLine, 10);
		} while (_stricmp(cmdLine, "N"));
	}

}


