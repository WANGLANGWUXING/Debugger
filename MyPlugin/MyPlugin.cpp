#include <windows.h>
#include <stdio.h>
#include <string.h>

//dll ��������  //��ʼ�����
extern "C"  bool _declspec(dllexport) InitPlugin(char plugin_name[250], DWORD *plugin_version)
{
	strcpy_s(plugin_name,250, "���2");
	*plugin_version = 0x01;
	return true;
}

//dll �������ܺ���
extern "C" bool _declspec(dllexport) DrawShape() {
	MessageBox(0, L"������һ�����", L"�������", 0);
	return true;

}