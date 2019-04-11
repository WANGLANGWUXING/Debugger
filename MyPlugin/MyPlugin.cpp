#include <windows.h>
#include <stdio.h>
#include <string.h>

//dll 导出函数  //初始化插件
extern "C"  bool _declspec(dllexport) InitPlugin(char plugin_name[250], DWORD *plugin_version)
{
	strcpy_s(plugin_name,250, "插件2");
	*plugin_version = 0x01;
	return true;
}

//dll 导出功能函数
extern "C" bool _declspec(dllexport) DrawShape() {
	MessageBox(0, L"这又是一个插件", L"插件标题", 0);
	return true;

}