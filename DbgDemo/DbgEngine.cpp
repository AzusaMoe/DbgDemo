#include "stdafx.h"
#include "DbgEngine.h"

/*******************************/
#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "Bea/headers/BeaEngine.h"
#pragma comment(lib, "Bea/Win32/Lib/BeaEngine.lib")
#pragma comment(linker, "/NODEFAULTLIB:\"crt.lib\"")
/*******************************/
#include <strsafe.h>

#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib") 


#define DBGOUT(format,error) \
printf("%s , %s , ��%d��: " ## format , __FILE__, __FUNCTION__ ,__LINE__,error)//��������ĺ�

CDbgEngine::CDbgEngine()
	:isSystemBp(TRUE),
	m_isUserTf(FALSE),
	m_isCcTf(FALSE),
	m_isBhTf(FALSE),
	m_isBmTf(FALSE),
	m_notWaitUser(FALSE)
{
	m_pTfBp = new CTfBp;
	m_pCcBp = new CCcBp;
	m_pLordPe = new CLordPe;
}


CDbgEngine::~CDbgEngine() {
	delete m_pTfBp; m_pTfBp = NULL;
	delete m_pCcBp; m_pCcBp = NULL;
	delete m_pLordPe; m_pLordPe = NULL;
}


//************************************
// FullName:  CDbgEngine::DebugMain
// Returns:   void
//************************************
void CDbgEngine::DebugMain() {
	//1.1	���Է�ʽ�򿪳���
	WCHAR szPath[] = L"CrackMe3.exe";
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	BOOL bStatus = CreateProcess(szPath, NULL, NULL, NULL, FALSE,
		DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,	//�����½����� | ӵ���¿���̨,���̳��丸������̨��Ĭ�ϣ�
		NULL, NULL, &si, &m_pi);
	if (!bStatus) {
		printf("�������Խ���ʧ��!\n");
		return;
	}
	//���ļ���ȡ���������ڴ棬�Ա����PE
	if (!m_pLordPe->GetDosHead(szPath))
	{
		DBGOUT("%s\n", "���ļ���ȡ���ڴ�ʧ�ܣ�");
	}
	m_dwOep = m_pLordPe->GetOep();//��ȡ�����Խ���OEP
	//1.2	��ʼ�������¼��ṹ��
	DEBUG_EVENT DbgEvent = { 0 };
	DWORD dwState = DBG_EXCEPTION_NOT_HANDLED;
	//2.�ȴ�Ŀ��Exe���������¼�
	while (1) {
		WaitForDebugEvent(&DbgEvent, INFINITE);
		//2.1 ���ݵ����¼�����,�ֱ���
		m_pDbgEvt = &DbgEvent;
		dwState = DispatchDbgEvent(DbgEvent);
		//2.2 �������쳣,�������б�����Exe
		ContinueDebugEvent(DbgEvent.dwProcessId, DbgEvent.dwThreadId, dwState);
	}
}

//************************************
// FullName:  CDbgEngine::DispatchDbgEvent
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::DispatchDbgEvent(DEBUG_EVENT& de) {
	//�жϵ�������
	DWORD dwRet = DBG_EXCEPTION_NOT_HANDLED;
	switch (de.dwDebugEventCode) {
	case CREATE_PROCESS_DEBUG_EVENT:	//���̵��� ֻ����һ��
		dwRet = OnCreateProcess(de);
		break;
	case EXCEPTION_DEBUG_EVENT:			//�쳣���ԣ��󲿷�ʱ�䶼�ķ�������
		dwRet = OnException(de);
		break;
	case CREATE_THREAD_DEBUG_EVENT:		//�̵߳���
	case EXIT_THREAD_DEBUG_EVENT:		//�˳��߳�
		break;
	case EXIT_PROCESS_DEBUG_EVENT:		//�˳�����
		dwRet = DBG_CONTINUE;
		break;
	case LOAD_DLL_DEBUG_EVENT:			//����DLL
		dwRet = OnLoadDll(de);					//printf("Load:%x\n", pDebugEvent->u.LoadDll.lpBaseOfDll); break;
		break;
	case UNLOAD_DLL_DEBUG_EVENT:		//ж��DLL
		OnUnLoadDll(de);								//printf("UnLoad:%x\n", pDebugEvent->u.UnloadDll.lpBaseOfDll); break;
		dwRet = DBG_CONTINUE;
		break;
	case OUTPUT_DEBUG_STRING_EVENT:		//��������ַ���
	case RIP_EVENT:						//RIP����
		return dwRet;	//������
	}
	return dwRet;
}

//************************************
// FullName:  CDbgEngine::OnCreateProcess
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::OnCreateProcess(DEBUG_EVENT& de) {
	// ���������Ϣ�������߳���Ϣ
	m_pi.dwProcessId = de.dwProcessId;
	m_pi.dwThreadId = de.dwThreadId;
	// ���̾��������ʹ��
	m_pi.hProcess = de.u.CreateProcessInfo.hProcess;
	// ����߳̾������ʹ��
	m_pi.hThread = de.u.CreateProcessInfo.hThread;
	//���汻���Խ��̻�ַ
	m_dwBaseAddr = (DWORD)de.u.CreateProcessInfo.lpBaseOfImage;
	// ��������ģ����Ϣ
	// .......��
	return DBG_CONTINUE;
}

//************************************
// FullName:  CDbgEngine::OnLoadDll
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::OnLoadDll(DEBUG_EVENT& de) {
	// ����ģ����Ϣ
	// ������Ϣ���в��ּ��ص�ģ����Ϣ
	LPLOAD_DLL_DEBUG_INFO lpDllInfo = &de.u.LoadDll;
	// ...������Ϣ�� 
	return DBG_CONTINUE;
}

//************************************
// FullName:  CDbgEngine::OnUnLoadDll
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::OnUnLoadDll(DEBUG_EVENT& de) {
	// DLL��ж�أ��ѱ����ģ����Ϣ���
	// de.u.UnloadDll������ж�ص�ģ���ַ
	// ����ͨ����ȥ�������ҵ����ĸ�DLL��ж��
	// ��
	return DBG_CONTINUE;
}

//************************************
// FullName:  CDbgEngine::OnException
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::OnException(DEBUG_EVENT& de) {
	// �����쳣���ͷֱ���������Ҫ�ж��ǲ����Լ��������쳣������ֱ�ӷ��ؾͺ�
	DWORD dwRet = DBG_EXCEPTION_NOT_HANDLED;
	switch (de.u.Exception.ExceptionRecord.ExceptionCode) {
		//����ϵ�
	case EXCEPTION_BREAKPOINT:
	{
		BOOL isMyBp = FALSE;
		for (auto each :m_bpAddrList[CC])//�ж��ǲ����Լ��µ�����ϵ�
		{
			if (each==(DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress)
			{
				isMyBp = TRUE;
				break;
			}
		}
		if (isSystemBp||isMyBp)
		{
			dwRet = OnExceptionCc(de);
		}
		
		break;
	}
		//TF��Ӳ���ϵ��쳣
	case EXCEPTION_SINGLE_STEP:
	{
		// ����쳣��Ϊʲô�����ģ�

		// �����Ӳ���ϵ㴥���ģ������û�ѡ��ĵ�����������һ��������Ȼ��ȴ��û����break��

		// �����Ϊ�˻ָ�����ϵ����õ�TF�ϵ�, �����û�ѡ��ĵ�������ֱ�ӻָ�����ϵ㣬Ȼ��return,��Ҫ���û�����

		// �����Ϊ�˻ָ�Ӳ���ϵ����õ�TF�ϵ㣬�����û�ѡ��ĵ�������ֱ�ӻָ�Ӳ���ϵ㣬Ȼ��return����Ҫ���û�����

		// ����û������ߣ�����Ϊ������Ŀ����û��TF�ϵ㣩����Ӧ��break�����Ϊ������Ŀ��������TF�ϵ㣬Ӧ����ִ����Ӧ��������break���������û�����

		if (m_BhBp.CheckDr6ForBhRwBreakPoint(de.dwThreadId))//�����Ӳ���ϵ㱻����
		{
			m_pTfBp->SetTfBreakPoint(de.dwThreadId);//����һ������
			m_isBhTf = TRUE;//��ʾ��һ��������Ϊ������Ӳ���ϵ�
			dwRet = DBG_CONTINUE;
			goto WaitUser;//һ��Ҫ�������switch������ȴ��û�����ĺ���������ᴥ�����������Ӳ���ϵ����
		}

		if (m_isCcTf)//��Ϊ�˻ָ�����ϵ�����õ�TF�ϵ�
		{
			m_isCcTf = FALSE;//ֻ�е���һ����������ϵ�ʱ��ֵ�ű���ΪTRUE
			//�ָ����жϵ㣬��Ϊ֮ǰ������ϵ��쳣����ȡ������������ϵ�
			if (!m_pCcBp->ReSetAllBsBreakPoint(m_pi.hProcess))
			{
				DBGOUT("%s\n", "�ָ���������ϵ�ʧ�ܣ�");
				break;
			}

			if (m_isUserTf)//�û�Ҳ������TF�ϵ㣬�����û�����
			{				
				m_notWaitUser = FALSE;//Ҫ�����û�����
			}
			else//�û�û��TF�ϵ㣬�������û�����
			{
				m_notWaitUser = TRUE;//�������û�����������ִ�г���
			}
		}

		if (m_isBhTf)//��Ϊ�˻ָ�Ӳ���ϵ�����õ�TF�ϵ�
		{
			m_isBhTf = FALSE;////ֻ�е���һ������Ӳ���ϵ�ʱ��ֵ�ű���ΪTRUE
			m_BhBp.ReSetAllBhRwBreakPoint(de.dwThreadId);//��������Ӳ���ϵ�

			if (m_isUserTf)//�û�ѡ�񵥲��ߣ������û�����
			{
				m_notWaitUser = FALSE;//Ҫ�����û�����
			}
			else//�û�ûѡ�񵥲��ߣ�û��TF�ϵ㣩���������û�����
			{
				m_notWaitUser = TRUE;//�������û�����������ִ�г���
			}
		}
		if (m_isBmTf)//��Ϊ�˻ָ��ڴ�ϵ�����õ�TF
		{
			m_bmBp.ReSetBmBreakPoint(m_pi.hProcess);
			m_notWaitUser = TRUE;//�������û�����������ִ�г���
		}
		m_isUserTf = FALSE;//ֻ�е���һ���û�����TF�ϵ��ֵ��Ϊ��
		dwRet = DBG_CONTINUE;
		break;
	}
		//�ڴ�����쳣
	case EXCEPTION_ACCESS_VIOLATION:
	{
		if ((DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress==m_bmAddr)//���ڴ�����쳣��ַ�������õĵ�ַʱ
		{
			dwRet = DBG_CONTINUE;
			break;//����switchȥ�����û�����
		}
		//�쳣��ַ�������õĵ�ַ��ȡ���ڴ�ϵ㣬���õ�����Ȼ��ֱ�ӷ��غ������������û�����
		m_bmBp.RemoveBmBreakPoint(m_pi.hProcess);
		m_pTfBp->SetTfBreakPoint(de.dwThreadId);
		m_isBmTf = TRUE;//��ʾ��һ�������ϵ���Ϊ�������ڴ�ϵ������
		return DBG_CONTINUE;//ֱ�ӷ��أ��������û�����
	}
	default:
		break;
	}
	if (m_notWaitUser)
	{
		m_notWaitUser = FALSE;
		return DBG_CONTINUE;//ֱ�ӷ��أ��������û�����
	}
WaitUser:
	WaitforUserCommand();
	return dwRet;
}

//************************************
// FullName:  CDbgEngine::OnExceptionCc
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::OnExceptionCc(DEBUG_EVENT& de) {
	if (isSystemBp)
	{
		isSystemBp = FALSE;//�����жϵ�һ��ϵͳ���õ�����ϵ㣬һ����
		return DBG_CONTINUE;//ֱ�ӷ��ؼ���ִ��
	}
	//1. ����������ϵ��ֵд��ȥ����ֹӰ�췴���,ͬʱ�������õ�ǰ����������ָ��ָ�ԭ״
	if (!m_pCcBp->RemoveAllBsBreakPoint(m_pi.hProcess))
	{
		DBGOUT("%s\n", "ȡ����������ϵ�ʧ�ܣ�");
	}
	if (!m_pCcBp->EipSubOne(de.dwThreadId))//EIP��һ
	{
		DBGOUT("%s\n", "EIP��һʧ��");
	}
	m_pTfBp->SetTfBreakPoint(de.dwThreadId);// ����1������
	m_isCcTf = TRUE;//Ϊ�˻ָ�����ϵ�����õĵ���
	return DBG_CONTINUE;
}



//************************************
// FullName:  CDbgEngine::ShowRegisterInfo
// Returns:   VOID
// Parameter: CONTEXT & ct
//************************************
VOID CDbgEngine::ShowRegisterInfo(CONTEXT& ct) {
	printf(
		"EAX = 0x%X\tEBX = 0x%X\tECX = 0x%X\tEDX = 0x%X\t\n"
		"ESP = 0x%X\tEBP = 0x%X\tESI = 0x%X\tEIP = 0x%X\t\n"
		"Dr0 = 0x%X\tDr1 = 0x%X\tDr2 = 0x%X\tDr3 = 0x%X\t\n",
		ct.Eax, ct.Ebx, ct.Ecx, ct.Edx, ct.Esp, ct.Ebp, ct.Esi, ct.Eip,
		ct.Dr0, ct.Dr1, ct.Dr2, ct.Dr3
	);
}

//************************************
// FullName:  CDbgEngine::WaitforUserCommand
// Returns:   DWORD
//************************************
VOID CDbgEngine::WaitforUserCommand() {
	// 1.����Ĵ�����Ϣ
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, 0, m_pDbgEvt->dwThreadId);
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// all register
	GetThreadContext(hThread, &ct);
	CloseHandle(hThread);
	ShowRegisterInfo(ct);
	// 2.����������Ϣ
	// ��!!!�쳣��ַ!!!��ʼ�����5����Ϣ����Ҫ��eip��ʼ
	DisasmAtAddr((DWORD)m_pDbgEvt->u.Exception.ExceptionRecord.ExceptionAddress);
	//3.����쳣������ַ
	printf("�쳣������ַ��%08X\n", (DWORD)m_pDbgEvt->u.Exception.ExceptionRecord.ExceptionAddress);
	// 3.�ȴ��û�����
	// �ȴ��û�����
	CHAR szCommand[MAX_INPUT] = {};
	while (1) {
		gets_s(szCommand, MAX_INPUT);
		switch (szCommand[0]) {
		case 'u':// ����� ���������û�����
			//UserCommandDisasm(szCommand);
			break;
		case 't':// ����F7
			m_pTfBp->SetTfBreakPoint(m_pDbgEvt->dwThreadId);
			m_isUserTf = TRUE;
			return;
		case 'p':// ����F8
			//UserCommandStepOver();
			return;
		case 'g':// go
			return;
		case 'b'://bs ����ϵ�; bm �ڴ�ϵ�; bh Ӳ���ϵ�; bl ��ѯ�ϵ��б�
			UserCommandB(szCommand);
			break;
		case 'k':// �鿴��������ջ֡
			//UserCommandK();
			break;
		case 'm':// �鿴ģ����Ϣ
			//UserCommandM();
			break;
		case 'd':
			//UserCommandD();// dump
			break;
		case 'o'://��OEP���¶�,���ó���ִ�е�OEP��
			m_pCcBp->SetBsBreakPoint(m_dwBaseAddr + m_dwOep, m_pi.hProcess);
			m_bpAddrList[CC].push_back(m_dwBaseAddr + m_dwOep);
			return;
		default:
			printf("��������ȷ��ָ�\n");
			break;
		}
	}
}



//************************************
// FullName:  CDbgEngine::UserCommandB
// Returns:   void
// Parameter: CHAR * pCommand
//************************************
void CDbgEngine::UserCommandB(CHAR* pCommand) {
	//  Bϵ������
	switch (pCommand[1]) {
	case 's':// bs ����ϵ�
	{
		string strTemp = pCommand;
		string strAddr = strTemp.substr(3, 8);//��ȡ����ַ
		int nAddr;
		StrToIntExA(strAddr.c_str(), STIF_SUPPORT_HEX,&nAddr);//תΪint��
		if (!m_pCcBp->SetBsBreakPoint((DWORD)nAddr, m_pi.hProcess))//��������ϵ�
		{
			DBGOUT("%s\n", "��������ϵ�ʧ�ܣ�");
		}
		m_bpAddrList[CC].push_back((DWORD)nAddr);//��¼����ϵ�
		break;
	}
	case 'h':// bh Ӳ���ϵ�
	{
		string strTemp = pCommand;
		string strAddr = strTemp.substr(3, 8);//��ȡ����ַ
		int nAddr;
		StrToIntExA(strAddr.c_str(), STIF_SUPPORT_HEX, &nAddr);//תΪint��
		string strType = strTemp.substr(12, 1);//��ȡ��Ӳ���ϵ�����
		string strLen = strTemp.substr(14, 1);//��ȡ��Ӳ���ϵ㳤��
		if (strcmp(strType.c_str(),"e")==0)//����Ӳ��ִ�жϵ�
		{
			if (!m_BhBp.SetBhExecBreakPoint(m_pDbgEvt->dwThreadId,(DWORD)nAddr))//����Ӳ��ִ�жϵ�
			{
				DBGOUT("%s\n", "Ӳ��ִ�жϵ�����ʧ�ܣ�");
			}
		}
		else//����д����д
		{
			int nType;
			if (strcmp(strType.c_str(), "r") == 0)
			{
				nType = R;//���ϵ�
			}
			if (strcmp(strType.c_str(), "w") == 0)
			{
				nType = W;//д�ϵ�
			}
			if (strcmp(strType.c_str(), "q") == 0)
			{
				nType = Q;//��д�ϵ�
			}
			if (strcmp(strLen.c_str(), "1") == 0)
			{
				if (!m_BhBp.SetBhRwBreakPoint(m_pDbgEvt->dwThreadId, (DWORD)nAddr, nType, OneByte))
				{
					DBGOUT("%s\n", "Ӳ����ȡ�ϵ�����ʧ�ܣ�");
				}
			}
			if (strcmp(strLen.c_str(), "2") == 0)
			{
				if (!m_BhBp.SetBhRwBreakPoint(m_pDbgEvt->dwThreadId, (DWORD)nAddr, nType, TwoByte))
				{
					DBGOUT("%s\n", "Ӳ��д��ϵ�����ʧ�ܣ�");
				}
			}
			if (strcmp(strLen.c_str(), "4") == 0)
			{
				if (!m_BhBp.SetBhRwBreakPoint(m_pDbgEvt->dwThreadId, (DWORD)nAddr, nType, FourByte))
				{
					DBGOUT("%s\n", "Ӳ����д�ϵ�����ʧ�ܣ�");
				}
			}
		}
		break;
	}
	case 'm':// bm�ڴ�ϵ�
	{
		string strTemp = pCommand;
		string strAddr = strTemp.substr(3, 8);//��ȡ����ַ
		int nAddr;
		StrToIntExA(strAddr.c_str(), STIF_SUPPORT_HEX, &nAddr);//תΪint��
		if (!m_bmBp.SetBmBreakPoint((DWORD)nAddr,m_pi.hProcess))
		{
			DBGOUT("%s\n", "�����ڴ���ʶϵ�ʧ�ܣ�");
		}
		m_bmAddr = nAddr;//�������õ��ڴ�ϵ��ַ
		break;
	}
	case 'l':// bl �鿴�ϵ��б�����
		//UserCommandBL(pCommand);
		break;
	default:
		printf("��������ȷ��ָ�\n");
		break;
	}
}

//************************************
// FullName:  CDbgEngine::UserCommandDisasm
// Returns:   void
// Parameter: CHAR * pCommand
//************************************
void CDbgEngine::UserCommandDisasm(CHAR* pCommand) {
	// ���������ָ�� u  ��ַ ���� ���ȿ�ʡ��
	char seps[] = " ";
	char *token = NULL;
	char *next_token = NULL;
	// token = 'u'
	token = strtok_s(pCommand, seps, &next_token);
	// ������ַ
	// token = address(123456)
	token = strtok_s(NULL, seps, &next_token);
	if (token == nullptr) {
		printf("��������ȷ��ָ�\n");
		return;
	}
	DWORD dwAddress = strtol(token, NULL, 16);
	//sscanf_s(token, "%8x", &dwAddress);
	if (!dwAddress) {
		printf("��������ȷ��ָ�\n");
		return;
	}
	// ���������
	DWORD dwCount = 10;
	// token = count(10)
	token = strtok_s(NULL, seps, &next_token);
	if (token != nullptr) {
		dwCount = strtol(token, NULL, 16);
		dwCount == 0 ? dwCount = 10 : dwCount;
	}
	DisasmAtAddr(dwAddress, dwCount);
}

//************************************
// FullName:  CDbgEngine::DisasmAtAddr
// Returns:   void
// Parameter: DWORD addr
// Parameter: DWORD dwCount
//************************************
void CDbgEngine::DisasmAtAddr(DWORD addr, DWORD dwCount/*= 10*/) {
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, m_pi.dwProcessId);
	WCHAR szOpCode[50] = {};
	WCHAR szAsm[50] = {};
	WCHAR szComment[50] = {};
	//2.3 һ�η����1��,Ĭ�Ϸ����10���������Զ��巴���ָ����Ŀ��Ҳ��������������ָ��
	printf("%-10s %-20s%-32s%s\n", "addr", "opcode", "asm", "comment");
	UINT uLen;
	for (DWORD i = 0; i < dwCount; i++) {
		// �����
		uLen = DBG_Disasm(hProcess, (LPVOID)addr, szOpCode, szAsm, szComment);
		wprintf_s(L"0x%08X %-20s%-32s%s\n", addr, szOpCode, szAsm, szComment);
		addr += uLen;
	}
	
	CloseHandle(hProcess);
}

//************************************
// FullName:  CDbgEngine::DBG_Disasm
// Returns:   UINT
// Parameter: HANDLE hProcess
// Parameter: LPVOID lpAddress
// Parameter: PWCHAR pOPCode
// Parameter: PWCHAR pASM
// Parameter: PWCHAR pComment
//************************************
UINT CDbgEngine::DBG_Disasm(HANDLE hProcess, LPVOID lpAddress, PWCHAR pOPCode, PWCHAR pASM, PWCHAR pComment) {
	// 1. �����Գ�����ڴ渴�Ƶ�����
	DWORD  dwRetSize = 0;
	BYTE lpRemote_Buf[32] = {};
	ReadProcessMemory(hProcess, lpAddress, lpRemote_Buf, 32, &dwRetSize);
	// 2. ��ʼ�����������
	DISASM objDiasm;
	objDiasm.EIP = (UIntPtr)lpRemote_Buf; // ��ʼ��ַ
	objDiasm.VirtualAddr = (UINT64)lpAddress;     // �����ڴ��ַ��������������ڼ����ַ��
	objDiasm.Archi = 0;                     // AI-X86
	objDiasm.Options = 0x000;                 // MASM
											  // 3. ��������
	UINT unLen = Disasm(&objDiasm);
	if (-1 == unLen) return unLen;
	// 4. ��������ת��Ϊ�ַ���
	LPWSTR lpOPCode = pOPCode;
	PBYTE  lpBuffer = lpRemote_Buf;
	for (UINT i = 0; i < unLen; i++) {
		StringCbPrintf(lpOPCode++, 50, L"%X", *lpBuffer & 0xF0);
		StringCbPrintf(lpOPCode++, 50, L"%X", *lpBuffer & 0x0F);
		lpBuffer++;
	}
	// 6. ���淴������ָ��
	WCHAR szASM[50] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, objDiasm.CompleteInstr, -1, szASM, _countof(szASM));
	StringCchCopy(pASM, 50, szASM);
	return unLen;
}


