// ע���HOOK�������������ָ������.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include <locale.h>

int main()
{
	// ���ÿ���̨���ַ���,Ϊ�˱���Unicode����������
	setlocale(LC_ALL, "chs");


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
	DWORD dwPid = 0;
	printf("������һ�������������PID: ");
	scanf_s("%d", &dwPid);

	// ���������������.
	HANDLE hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
		FALSE,
		dwPid);
	if (hProc == INVALID_HANDLE_VALUE)
	{
		printf("��ʧ��,������Ȩ�޲���\n");
		return 0;
	}

	char dllPath[MAX_PATH] = "D:\\Code\\HookOpenProcess.dll";
	//printf("����Ҫע���DLL·��: ");
	//gets_s(dllPath, MAX_PATH);

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


	// �����ļ�ӳ��,׼�����н��̼��ͨѶ
	HANDLE hFilMap = CreateFileMapping(INVALID_HANDLE_VALUE,
		0,
		PAGE_READWRITE,
		0,
		4096,
		L"Global\\HOOKTASKMGR");
	LPVOID pProcessName = 0;
	pProcessName = MapViewOfFile(hFilMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 4096);
	printf("������Ҫ����Ľ��̵�����: ");
	getchar();
	_getws_s((wchar_t*)pProcessName, MAX_PATH);


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
	UnmapViewOfFile(pProcessName);
	CloseHandle(hFilMap);

	return 0;
}

