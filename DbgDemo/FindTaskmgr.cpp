#include "stdafx.h"
#include "FindTaskmgr.h"
#include <Shlwapi.h>

#define DBGOUT(format,error) \
printf("%s , %s , ��%d��: " ## format , __FILE__, __FUNCTION__ ,__LINE__,error)//��������ĺ�

CFindTaskmgr::CFindTaskmgr()
{
}


CFindTaskmgr::~CFindTaskmgr()
{
}


void CFindTaskmgr::CreatDetectionThread()
{
	CreateThread(NULL, NULL, CycleDetectionTaskmgr, (LPVOID)this, NULL, NULL);
}


DWORD WINAPI CFindTaskmgr::CycleDetectionTaskmgr(LPVOID lpParam)
{
	CFindTaskmgr *pThis = (CFindTaskmgr*)lpParam;
	if (pThis->OpenDllInjector())
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	
}

BOOL CFindTaskmgr::OpenDllInjector()
{
	while (true)
	{
		// ����
		HANDLE hProcessSnap;//���̿��վ��
		PROCESSENTRY32 stcPe32 = { 0 };//���̿�����Ϣ
		stcPe32.dwSize = sizeof(PROCESSENTRY32);
		//1.����һ��������صĿ��վ��
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE) return FALSE;
		//2ͨ�����̿��վ����ȡ��һ��������Ϣ
		if (!Process32First(hProcessSnap, &stcPe32)) {
			CloseHandle(hProcessSnap);
			return FALSE;
		}
		//3ѭ������������Ϣ
		do {
			//3.1��ȡ������
			if (StrCmpW(stcPe32.szExeFile, L"Taskmgr.exe") == 0)
			{
				char temp[10] = { 0 };
				sprintf(temp, "%d", stcPe32.th32ProcessID);
				if (!WriteTOFile(L"TaskmgrPID.txt", temp))
				{
					-------------------------------
				}
				//STARTUPINFO si = { sizeof(STARTUPINFO) };
				//PROCESS_INFORMATION pi;
				//if (CreateProcess(L"DllInjector.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
				//{
				//	return TRUE;
				//}
				//else
				//{
				//	return FALSE;
				//}
			}
		} while (Process32Next(hProcessSnap, &stcPe32));
		CloseHandle(hProcessSnap);
		Sleep(1000);
	}
}


BOOL CFindTaskmgr::WriteTOFile(WCHAR* strPath, char* szContext)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFile(strPath,	/* �ļ�·�� */
		GENERIC_READ | GENERIC_WRITE,	/*���ʷ�ʽ*/
		0,								/*�ļ�����ʽ*/
		NULL,							/*��ȫ������*/
		OPEN_EXISTING,					/*�ļ�������־*/
		FILE_ATTRIBUTE_NORMAL,			/*�ļ���־������*/
		NULL							/*ģ����,Ĭ����NULL*/
	);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("���ļ�ʧ��,�ļ�������\n");
		return FALSE;
	}
	int size = strlen(szContext);
	szContext[size] = '\r';
	szContext[size + 1] = '\n';
	szContext[size + 2] = '\0';
	// �����ļ���дλ��
	SetFilePointer(hFile, 0, 0, FILE_END);
	// д���ļ�.
	DWORD dwWrite = 0;
	WriteFile(hFile,
		szContext,					/*Ҫд��Ļ��������׵�ַ*/
		strlen(szContext),			/*Ҫд�뵽�ļ��е��ֽ���*/
		&dwWrite,					/*ʵ��д����ֽ���*/
		NULL);

	CloseHandle(hFile);
	return TRUE;
}
