#include "stdafx.h"
#include "TfBp.h"


CTfBp::CTfBp()
{
}


CTfBp::~CTfBp()
{
}

//�����ߣ�����TF�ϵ㣩
void CTfBp::UserCommandStepInto(DWORD dwThreadId) {
	// ���õ���
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, NULL, dwThreadId);
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// ָ��Ҫ��ȡ��д�Ĵ�������Ϣ������Ҫ
	GetThreadContext(hThread, &ct);
	PEFLAGS pElg = (PEFLAGS)&ct.EFlags;
	pElg->TF = 1;
	SetThreadContext(hThread, &ct);
	CloseHandle(hThread);
}