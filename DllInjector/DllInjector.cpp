// ע���HOOK�������������ָ������.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include <locale.h>
#include<fstream>
#include<iostream>  
#include <string>
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib") 
using std::string;
using std::ifstream;

int main()
{
	// ���ÿ���̨���ַ���,Ϊ�˱���Unicode����������
	//setlocale(LC_ALL, "chs");

	// ������һ��DLL , ��DLL����HOOK�����������
	// OpenProcess�����Ĵ���. �����б���ָ������
	// �����رյĴ���.

	// ��ǰ��Ŀ�Ĺ���: ��һ��DLLע�뵽�����������.
	// Ҫ���ע��Ĺ���, ��Ҫ��������:
	// 1. �õ�����������Ľ��̾��.����,�ý��̾��
	//    ��Ҫ���洴���߳�,��д�����ڴ��Ȩ��.
	// 2. ��DLL��·��д�뵽��������������ڴ���,����.
	// 3. ����һ��Զ���߳�, ���̵߳Ļص���������ΪLoadLibrary
	//    ���̵߳ĸ��Ӳ�������Ϊ����������������ڴ��е�
	//    DLL�ļ�·��.

	string buf;
	ifstream infile;
	infile.open(_T("..\\DbgDemo\\TaskmgrPID.txt"));
	if (!infile) {
		return 0;
	}
	//���ж�ȡֱ����ȡ���
	getline(infile, buf);
	infile.close();
	DWORD dwPid = 0;
	dwPid = StrToIntA(buf.c_str());


	// ���������������.
	HANDLE hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE,FALSE,dwPid);
	if (hProc == INVALID_HANDLE_VALUE)
	{
		printf("��ʧ��,������Ȩ�޲���\n");
		return 0;
	}

	char dllPath[MAX_PATH] = "D:\\Code\\DbgDemo\\x64\\Debug\\HookOpenProcess.dll";

	// 2. ��dll·��д�뵽����������Ľ����ڴ���.
	LPVOID pBuff = NULL;
	pBuff = VirtualAllocEx(hProc,/*���̾��*/
		NULL,/*ָ���ĵ�ַ.*/
		4096,/*����Ĵ�С*/
		MEM_RESERVE | MEM_COMMIT,/*�ڴ��״̬*/
		PAGE_READWRITE /*�ڴ��ҳ����*/);
	if (pBuff == NULL)
	{
		printf("�����ڴ�ʧ��\n");
		return 0;
	}

	// 3. ��dll·��д�뵽��������ڴ���
	SIZE_T dwWrite = 0;
	WriteProcessMemory(hProc,  /*���̾��*/
		pBuff,  /*Ҫд��ĵ�ַ*/
		dllPath, /*Ҫд�뻺����*/
		strlen(dllPath) + 1, /*���������ֽ���*/
		&dwWrite/*����ʵ��д����ֽ���*/);

	// 4. ����Զ���߳�
	//    Ŀ��: Ϊ����Զ�̽����е���LoadLibrary
	HANDLE hThread = INVALID_HANDLE_VALUE;
	hThread = CreateRemoteThread(hProc, /*���̾��*/
		NULL, /*��ȫ������*/
		0,/*�߳�ջ���ֽ���*/
		(LPTHREAD_START_ROUTINE)&LoadLibraryA,/*�̵߳Ļص�����*/
		pBuff,/*�̻߳ص������ĸ��Ӳ���*/
		0,
		0);
	// �ȴ��߳��˳�.
	// ��Ҫ�ȴ�LoadLibrary�����Ľ���.
	WaitForSingleObject(hThread, -1);

	// �ͷ�Զ�̽��̵Ŀռ�.
	VirtualFreeEx(hProc, pBuff, 0, MEM_RELEASE);

	return 0;
}
