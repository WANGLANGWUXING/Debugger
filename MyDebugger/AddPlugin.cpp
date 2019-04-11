#include "AddPlugin.h"
#include <windows.h>
#include <vector>
using std::vector;
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
//1. exe目录下新建一个plugin文件夹
//2. 加载插件程序 - 显示插件信息
//3. 遍历plugin下所以dll文件
//4. 动态加载dll
//5. 获取插件相关信息
//6. 指定位置调用插件内部函数


//dll 导出函数
//bool InitPlugin(char plugin_name[250], DWORD *plugin_version);
//dll 导出功能函数
//bool DrawShape();

typedef bool(*InitPlugin)(char *, DWORD *);
typedef bool(*DrawShape)();



//插件信息
typedef struct _MYPLUGIN {
	char plugin_name[250];
	DWORD plugin_vesrion;
	HMODULE plugin_hmodule;
	DrawShape pfunc;
}MYPLUGIN, *PMYPLUGIN;

//插件容器
vector<MYPLUGIN> g_vecPlugin;

void LoadPlugin()
{
	printf("插件加载中......\n");
	//遍历plugin下所以dll
	WIN32_FIND_DATAA wfd = {};
	HANDLE hFile = FindFirstFileA(".\\plugin\\*.dll", &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("插件加载失败\n");
		return;
	}
	do
	{
		MYPLUGIN myplugin;
		//拼接dll路径
		char dllname[250];
		sprintf_s(dllname, ".\\plugin\\%s", wfd.cFileName);
		//获取后缀名
		if (strcmp(PathFindExtensionA(wfd.cFileName), ".dll") != 0)
		{
			continue;
		}

		//加载dll
		HMODULE hModule = LoadLibraryA(dllname);
		if (hModule == NULL)
		{
			continue;
		}

		//获取导出的函数
		InitPlugin pFunc = (InitPlugin)GetProcAddress(hModule, "InitPlugin");
		//获取插件 名字，插件版本
		pFunc(myplugin.plugin_name, &myplugin.plugin_vesrion);
		myplugin.plugin_hmodule = hModule;
		//获取插件功能函数
		myplugin.pfunc = (DrawShape)GetProcAddress(hModule, "DrawShape");
		g_vecPlugin.push_back(myplugin);

	} while (FindNextFileA(hFile, &wfd));

	printf("插件加载成功\n");
}

void PluginInit()
{
	LoadPlugin();
	if (g_vecPlugin.size() > 0)
	{
		printf("序号\t插件名\n");
		//显示插件名字
		for (int i = 0; i < g_vecPlugin.size(); i++)
		{
			printf("%d\t%s\n", i + 1, g_vecPlugin[i].plugin_name);
		}

		//调用插件功能
		DWORD dwSel;
		char cmdLine[10];
		do
		{
			printf("请输入插件序号：");
			scanf_s("%d", &dwSel);
			dwSel = dwSel - 1;
			if (dwSel < g_vecPlugin.size() && dwSel >= 0)
			{
				g_vecPlugin[dwSel].pfunc();
			}
			else {
				printf("序号错误，请重新输入.\n");
				continue;
			}
			printf("是否继续？(Y/N)");
			scanf_s("%s", cmdLine, 10);
		} while (_stricmp(cmdLine, "N"));
	}

}


