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
#include <Psapi.h>.



#define DBGOUT(format,error) \
printf("%s , %s , 第%d行: " ## format , __FILE__, __FUNCTION__ ,__LINE__,error)//用来报错的宏

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
	//1.1	调试方式打开程序
	WCHAR m_szPath[] = L"CrackMe3.exe";
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	BOOL bStatus = CreateProcess(m_szPath, NULL, NULL, NULL, FALSE,
		DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,	//调试新建进程 | 拥有新控制台,不继承其父级控制台（默认）
		NULL, NULL, &si, &m_pi);
	if (!bStatus) {
		printf("创建调试进程失败!\n");
		return;
	}
	//将文件读取进本程序内存，以便解析PE
	if (!m_pLordPe->GetDosHead(m_szPath))
	{
		DBGOUT("%s\n", "将文件读取进内存失败！");
	}
	m_dwOep = m_pLordPe->GetOep();//获取被调试进程OEP

	//监测任务管理器，一旦任务管理器被打开就注入任务管理器
	m_findTaskmgr.CreatDetectionThread();

	//1.2	初始化调试事件结构体
	DEBUG_EVENT DbgEvent = { 0 };
	DWORD dwState = DBG_EXCEPTION_NOT_HANDLED;
	//2.等待目标Exe产生调试事件
	while (1) {
		WaitForDebugEvent(&DbgEvent, INFINITE);
		//2.1 根据调试事件类型,分别处理
		m_pDbgEvt = &DbgEvent;
		dwState = DispatchDbgEvent(DbgEvent);
		//2.2 处理完异常,继续运行被调试Exe
		ContinueDebugEvent(DbgEvent.dwProcessId, DbgEvent.dwThreadId, dwState);
	}
}

//************************************
// FullName:  CDbgEngine::DispatchDbgEvent
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::DispatchDbgEvent(DEBUG_EVENT& de) {
	//判断调试类型
	DWORD dwRet = DBG_EXCEPTION_NOT_HANDLED;
	switch (de.dwDebugEventCode) {
	case CREATE_PROCESS_DEBUG_EVENT:	//进程调试 只调用一次
		dwRet = OnCreateProcess(de);
		break;
	case EXCEPTION_DEBUG_EVENT:			//异常调试，大部分时间都耗费在这了
		dwRet = OnException(de);
		break;
	case CREATE_THREAD_DEBUG_EVENT:		//线程调试
	case EXIT_THREAD_DEBUG_EVENT:		//退出线程
		break;
	case EXIT_PROCESS_DEBUG_EVENT:		//退出进程
		dwRet = DBG_CONTINUE;
		break;
	case LOAD_DLL_DEBUG_EVENT:			//加载DLL
		dwRet = OnLoadDll(de);			//printf("Load:%x\n", pDebugEvent->u.LoadDll.lpBaseOfDll); break;
		break;
	case UNLOAD_DLL_DEBUG_EVENT:		//卸载DLL
		OnUnLoadDll(de);				//printf("UnLoad:%x\n", pDebugEvent->u.UnloadDll.lpBaseOfDll); break;
		dwRet = DBG_CONTINUE;
		break;
	case OUTPUT_DEBUG_STRING_EVENT:		//输出调试字符串
	case RIP_EVENT:						//RIP调试
		return dwRet;	//不处理
	}
	return dwRet;
}

//************************************
// FullName:  CDbgEngine::OnCreateProcess
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::OnCreateProcess(DEBUG_EVENT& de) {
	// 保存进程信息，和主线程信息
	m_pi.dwProcessId = de.dwProcessId;
	m_pi.dwThreadId = de.dwThreadId;
	// 进程句柄，放心使用
	m_pi.hProcess = de.u.CreateProcessInfo.hProcess;
	// 这个线程句柄谨慎使用
	m_pi.hThread = de.u.CreateProcessInfo.hThread;
	//保存被调试进程基址
	m_dwBaseAddr = (DWORD)de.u.CreateProcessInfo.lpBaseOfImage;
	// 保存下主模块信息
	// .......略
	return DBG_CONTINUE;
}

//************************************
// FullName:  CDbgEngine::OnLoadDll
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::OnLoadDll(DEBUG_EVENT& de) {
	// 保存模块信息
	// 调试信息里有部分加载的模块信息
	MyLOAD_DLL_DEBUG_INFO myLoadDllInfoTemp;
	myLoadDllInfoTemp.hFile = de.u.LoadDll.hFile;
	myLoadDllInfoTemp.lpBaseOfDll = de.u.LoadDll.lpBaseOfDll;
	myLoadDllInfoTemp.dwDebugInfoFileOffset = de.u.LoadDll.dwDebugInfoFileOffset;
	myLoadDllInfoTemp.nDebugInfoSize = de.u.LoadDll.nDebugInfoSize;
	myLoadDllInfoTemp.lpImageName = de.u.LoadDll.lpImageName;
	myLoadDllInfoTemp.fUnicode = de.u.LoadDll.fUnicode;
	m_vecLoadDllInfo.push_back(myLoadDllInfoTemp);
	return DBG_CONTINUE;
}

//************************************
// FullName:  CDbgEngine::OnUnLoadDll
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::OnUnLoadDll(DEBUG_EVENT& de) {
	// DLL被卸载，把保存的模块信息清除
	// de.u.UnloadDll保存有卸载的模块基址
	// 可以通过他去数组中找到是哪个DLL被卸载
	// 略
	for (DWORD i = 0; i < m_vecLoadDllInfo.size(); i++)
	{
		if (m_vecLoadDllInfo[i].lpBaseOfDll==de.u.UnloadDll.lpBaseOfDll)
		{
			m_vecLoadDllInfo.erase(m_vecLoadDllInfo.begin() + i);//删除向量中的当前对象
			break;
		}

	}
	return DBG_CONTINUE;
}

//************************************
// FullName:  CDbgEngine::OnException
// Returns:   DWORD
// Parameter: DEBUG_EVENT & de
//************************************
DWORD CDbgEngine::OnException(DEBUG_EVENT& de) {
	// 根据异常类型分别处理，在这里要判断是不是自己引发的异常，不是直接返回就好
	DWORD dwRet = DBG_EXCEPTION_NOT_HANDLED;
	switch (de.u.Exception.ExceptionRecord.ExceptionCode) {
	default:
		break;
	//软件断点
	case EXCEPTION_BREAKPOINT:
	{
		BOOL isMyBp = FALSE;
		for (auto each :m_bpAddrList[CC])//判断是不是自己下的软件断点
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
			if (dwRet == -1)//说明条件断点不满足，因此此时不应该接受用户命令，而是让程序继续执行
			{
				return DBG_CONTINUE;
			}
		}
		
		break;
	}
	//内存访问异常
	case EXCEPTION_ACCESS_VIOLATION:
	{
		if ((DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress==m_bmAddr)//当内存访问异常地址等于设置的地址时
		{
			m_bmBp.RemoveBmBreakPoint(m_pi.hProcess);//内存断点命中，取消内存断点
			dwRet = DBG_CONTINUE;
			break;//跳出switch去接收用户输入
		}
		//异常地址不是设置的地址，取消内存断点，设置单步，然后直接返回函数，不接受用户输入
		m_bmBp.RemoveBmBreakPoint(m_pi.hProcess);
		m_pTfBp->SetTfBreakPoint(de.dwThreadId);
		m_isBmTf = TRUE;//表示下一个单步断点是为了重设内存断点而设置
		return DBG_CONTINUE;//直接返回，不接受用户输入
	}
	//TF或硬件断点异常
	case EXCEPTION_SINGLE_STEP:
	{
		// 这个异常是为什么触发的？

		// 如果是硬件断点触发的，不是用户选择的单步，则设置一个单步，然后等待用户命令（break）

		// 如果是为了恢复软件断点设置的TF断点, 不是用户选择的单步，则直接恢复软件断点，然后return,不要等用户命令

		// 如果是为了恢复硬件断点设置的TF断点，不是用户选择的单步，则直接恢复硬件断点，然后return，不要等用户命令

		// 如果用户单步走（不管为了其它目的设没设TF断点），都应该break（如果为了其它目的有设置TF断点，应该先执行相应操作，再break），接收用户命令

		if (m_BhBp.CheckDr6ForBhRwBreakPoint(de.dwThreadId))//如果是硬件断点被触发
		{
			m_pTfBp->SetTfBreakPoint(de.dwThreadId);//设置一个单步
			m_isBhTf = TRUE;//表示下一个单步是为了重设硬件断点
			dwRet = DBG_CONTINUE;
			goto WaitUser;//一定要跳出外层switch，进入等待用户命令的函数，否则会触发下面的重设硬件断点操作
		}

		if (m_isCcTf)//是为了恢复软件断点而设置的TF断点
		{
			m_isCcTf = FALSE;//只有当下一次设置软件断点时此值才被设为TRUE
			//恢复所有断点，因为之前在软件断点异常处理处取消了所有软件断点
			if (!m_pCcBp->ReSetAllBsBreakPoint(m_pi.hProcess))
			{
				DBGOUT("%s\n", "恢复所有软件断点失败！");
				break;
			}

			if (m_isUserTf)//用户也设置了TF断点，接收用户输入
			{				
				m_notWaitUser = FALSE;//要接受用户操作
			}
			else//用户没设TF断点，不接受用户输入
			{
				m_notWaitUser = TRUE;//不接受用户操作，继续执行程序
			}
		}

		if (m_isBhTf)//是为了恢复硬件断点而设置的TF断点
		{
			m_isBhTf = FALSE;//只有当下一次设置硬件断点时此值才被设为TRUE
			m_BhBp.ReSetAllBhRwBreakPoint(de.dwThreadId);//重设所有硬件断点

			if (m_isUserTf)//用户选择单步走，接收用户输入
			{
				m_notWaitUser = FALSE;//要接受用户操作
			}
			else//用户没选择单步走（没设TF断点），不接受用户输入
			{
				m_notWaitUser = TRUE;//不接受用户操作，继续执行程序
			}
		}
		if (m_isBmTf)//是为了恢复内存断点而设置的TF
		{
			m_isBmTf = FALSE; //只有当下一次设置内存断点时此值才被设为TRUE
			m_bmBp.ReSetBmBreakPoint(m_pi.hProcess);
			m_notWaitUser = TRUE;//不接受用户操作，继续执行程序
		}

		m_isUserTf = FALSE;//只有当下一次用户设置TF断点此值才为真
		dwRet = DBG_CONTINUE;
		break;
	}
	}
	//是否接受用户操作的关键点
	if (m_notWaitUser)
	{
		m_notWaitUser = FALSE;
		return DBG_CONTINUE;//直接返回，不接受用户输入
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
		isSystemBp = FALSE;//用于判断第一个系统设置的软件断点，一次性
		return DBG_CONTINUE;//直接返回继续执行
	}
	BOOL isSatisfaction = TRUE;

	for (auto each : m_bpAddrList[BP])//判断是不是自己下的条件断点
	{
		if (each == (DWORD)de.u.Exception.ExceptionRecord.ExceptionAddress)
		{
			//是条件断点再判断条件是否满足
			isSatisfaction = IsConditionBreakPoint(de.dwThreadId);

		}
	}
	//1. 把所有软件断点的值写回去，防止影响反汇编,同时还可以让当前被断下来的指令恢复原状
	if (!m_pCcBp->RemoveAllBsBreakPoint(m_pi.hProcess))
	{
		DBGOUT("%s\n", "取消所有软件断点失败！");
	}
	if (!m_pCcBp->EipSubOne(de.dwThreadId))//EIP减一
	{
		DBGOUT("%s\n", "EIP减一失败");
	}
	m_pTfBp->SetTfBreakPoint(de.dwThreadId);// 设置1个单步
	m_isCcTf = TRUE;//为了恢复软件断点而设置的单步
	if (isSatisfaction)
	{
		return DBG_CONTINUE;
	}
	else
	{
		return -1;//特殊情况，表示条件不满足
	}
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
	// 1.输出寄存器信息
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, 0, m_pDbgEvt->dwThreadId);
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// all register
	GetThreadContext(hThread, &ct);
	CloseHandle(hThread);
	ShowRegisterInfo(ct);
	// 2.输出反汇编信息
	// 从!!!异常地址!!!开始反汇编5行信息，不要从eip开始
	DisasmAtAddr((DWORD)m_pDbgEvt->u.Exception.ExceptionRecord.ExceptionAddress);
	//3.输出异常触发地址
	printf("异常触发地址：%08X\n", (DWORD)m_pDbgEvt->u.Exception.ExceptionRecord.ExceptionAddress);
	// 3.等待用户命令
	// 等待用户命令
	CHAR szCommand[MAX_INPUT] = {};
	while (1) {
		gets_s(szCommand, MAX_INPUT);
		switch (szCommand[0]) {
		case 'u':// 反汇编 继续接受用户命令
			//UserCommandDisasm(szCommand);
			break;
		case 's'://插入机器码
		{
			unsigned char* szOpcode = NULL;//汇编出来的机器码
			size_t nOpcodeSize;//输出的opcode字节数
			int nAddr;//要插入指令的首地址
			if (!m_compilation.GetOpcode(m_pi.hProcess))
			{
				DBGOUT("%s\n", "写入汇编指令失败！");
			}
			break;
		}
		case 't':// 单步F7
			m_pTfBp->SetTfBreakPoint(m_pDbgEvt->dwThreadId);
			m_isUserTf = TRUE;
			return;
		case 'p':// 单步F8
			//UserCommandStepOver();
			return;
		case 'g':// go
			return;
		case 'b'://bs 软件断点; bm 内存断点; bh 硬件断点; bl 查询断点列表
			UserCommandB(szCommand);
			break;
		case 'k':// 查看函数调用栈帧
			if (!GetStackInfo(m_pDbgEvt->dwThreadId, m_pi.hProcess))
			{
				DBGOUT("%s\n", "查看堆栈信息失败！");
			}
			break;
		case 'm':// 查看模块信息
			EnumModules(m_pi.dwProcessId);
			PrintfModulesInfo();
			break;
		case 'i'://查看导入表信息
		{
			m_pLordPe->ImportTable();//解析导入表
			PrintfImportDll();//输出导入表基本信息
			printf("请输入dll名查看详细信息：");
			WCHAR temp[MAX_PATH] = { 0 };
			wscanf(L"%s", temp);
			CString dllName = temp;
			int i = 0;//计数
			for (auto each:m_pLordPe->m_vecImportDescriptor)
			{
				if (each.Name==dllName)
				{
					break;
				}
				++i;
			}
			printf("%-10s\t%-10s\n", "序号", "名称");
			for (auto& each : m_pLordPe->m_vvImportFunInfo[i])
			{
				wprintf(L"%08X\t%s\n", each.Ordinal, each.Name);
			}
			break;
		}
		case 'e'://查看导出表信息
		{
			EnumModules(m_pi.dwProcessId);
			PrintfModulesInfo();
			printf("请输入dll名查看其导出表：");
			WCHAR temp[MAX_PATH] = { 0 };
			wscanf(L"%s", temp);
			CString dllName = temp;
			CString strModuleName;
			for (auto each : m_vecModule)
			{
				if (each.szModule == dllName)
				{
					strModuleName = each.szExePath;
					break;
				}
			}
			m_pLordPe->GetDosHead(strModuleName);
			m_pLordPe->ExportTable();

			printf("名称：%s\n", m_pLordPe->m_my_im_ex_di.name);
			printf("序号基数：%08X\n", m_pLordPe->m_my_im_ex_di.Base);
			printf("函数数量：%08X\n", m_pLordPe->m_my_im_ex_di.NumberOfFunctions);
			printf("函数名数量%08X\n", m_pLordPe->m_my_im_ex_di.NumberOfNames);
			printf("地址表RVA%08X\n", m_pLordPe->m_my_im_ex_di.AddressOfFunctions);
			printf("名称表RVA%08X\n", m_pLordPe->m_my_im_ex_di.AddressOfNames);
			printf("序号表RVA%08X\n", m_pLordPe->m_my_im_ex_di.AddressOfNameOrdinals);

			printf("%-10s\t%-10s\t%-10s\t%-10s\n", "导出序号", "RVA", "偏移", "函数名");
			for (EXPORTFUNINFO &each : m_pLordPe->m_vecExportFunInfo)
			{
				wprintf(L"%08X\t%08X\t%08X\t%s\n", each.ExportOrdinals, each.FunctionRVA, each.FunctionOffset, each.FunctionName);
			}
			break;
		}
		case 'o'://在OEP处下断,并让程序执行到OEP处
			m_pCcBp->SetBsBreakPoint(m_dwBaseAddr + m_dwOep, m_pi.hProcess);
			m_bpAddrList[CC].push_back(m_dwBaseAddr + m_dwOep);
			return;
		default:
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
	//  B系列命令
	switch (pCommand[1]) {
	case 's':// bs 软件断点
	{
		string strTemp = pCommand;
		string strAddr = strTemp.substr(3, 8);//截取出地址
		int nAddr;
		StrToIntExA(strAddr.c_str(), STIF_SUPPORT_HEX,&nAddr);//转为int型
		if (!m_pCcBp->SetBsBreakPoint((DWORD)nAddr, m_pi.hProcess))//设置软件断点
		{
			DBGOUT("%s\n", "设置软件断点失败！");
		}
		m_bpAddrList[CC].push_back((DWORD)nAddr);//记录软件断点
		break;
	}
	case 'h':// bh 硬件断点
	{
		string strTemp = pCommand;
		string strAddr = strTemp.substr(3, 8);//截取出地址
		int nAddr;
		StrToIntExA(strAddr.c_str(), STIF_SUPPORT_HEX, &nAddr);//转为int型
		string strType = strTemp.substr(12, 1);//截取出硬件断点类型
		string strLen = strTemp.substr(14, 1);//截取出硬件断点长度
		if (strcmp(strType.c_str(),"e")==0)//设置硬件执行断点
		{
			if (!m_BhBp.SetBhExecBreakPoint(m_pDbgEvt->dwThreadId,(DWORD)nAddr))//设置硬件执行断点
			{
				DBGOUT("%s\n", "硬件执行断点设置失败！");
			}
		}
		else//读，写，读写
		{
			int nType;
			if (strcmp(strType.c_str(), "r") == 0)
			{
				nType = R;//读断点
			}
			if (strcmp(strType.c_str(), "w") == 0)
			{
				nType = W;//写断点
			}
			if (strcmp(strType.c_str(), "q") == 0)
			{
				nType = Q;//读写断点
			}
			if (strcmp(strLen.c_str(), "1") == 0)
			{
				if (!m_BhBp.SetBhRwBreakPoint(m_pDbgEvt->dwThreadId, (DWORD)nAddr, nType, OneByte))
				{
					DBGOUT("%s\n", "硬件读取断点设置失败！");
				}
			}
			if (strcmp(strLen.c_str(), "2") == 0)
			{
				if (!m_BhBp.SetBhRwBreakPoint(m_pDbgEvt->dwThreadId, (DWORD)nAddr, nType, TwoByte))
				{
					DBGOUT("%s\n", "硬件写入断点设置失败！");
				}
			}
			if (strcmp(strLen.c_str(), "4") == 0)
			{
				if (!m_BhBp.SetBhRwBreakPoint(m_pDbgEvt->dwThreadId, (DWORD)nAddr, nType, FourByte))
				{
					DBGOUT("%s\n", "硬件读写断点设置失败！");
				}
			}
		}
		break;
	}
	case 'm':// bm内存断点
	{
		string strTemp = pCommand;
		string strAddr = strTemp.substr(3, 8);//截取出地址
		int nAddr;
		StrToIntExA(strAddr.c_str(), STIF_SUPPORT_HEX, &nAddr);//转为int型
		if (!m_bmBp.SetBmBreakPoint((DWORD)nAddr,m_pi.hProcess))
		{
			DBGOUT("%s\n", "设置内存访问断点失败！");
		}
		m_bmAddr = nAddr;//保存设置的内存断点地址
		break;
	}
	case 'p'://条件断点
	{
		string strTemp = pCommand;
		string strAddr = strTemp.substr(3, 8);//截取出地址
		int nAddr;
		StrToIntExA(strAddr.c_str(), STIF_SUPPORT_HEX, &nAddr);//转为int型
		if (!m_pCcBp->SetBsBreakPoint((DWORD)nAddr, m_pi.hProcess))//设置软件断点
		{
			DBGOUT("%s\n", "设置软件断点失败！");
		}
		m_bpAddrList[BP].push_back((DWORD)nAddr);//记录条件断点
		m_bpAddrList[CC].push_back((DWORD)nAddr);//同时记录在软件断点里面

		string strLeft = strTemp.substr(12, 3);//截取出表达式左边的寄存器
		string strSymbol = strTemp.substr(15, 2);//截取出表达式的操作符
		string strRight = strTemp.substr(17, strTemp.size() - 17 + 1);//截取出表达式右边的操作数
		m_dwRight = StrToIntA(strRight.c_str());
		if (strcmp(strLeft.c_str(), "eax") == 0)
		{
			m_dwLeft = EAX;
		}
		if (strcmp(strLeft.c_str(), "ebx") == 0)
		{
			m_dwLeft = EBX;
		}
		if (strcmp(strLeft.c_str(), "ecx") == 0)
		{
			m_dwLeft = ECX;
		}
		if (strcmp(strLeft.c_str(), "edx") == 0)
		{
			m_dwLeft = ECX;
		}
		if (strcmp(strSymbol.c_str(), "> ") == 0)
		{
			m_dwSymbol = MoreThan;
		}
		if (strcmp(strSymbol.c_str(), "==") == 0)
		{
			m_dwSymbol = Equal;
		}
		if (strcmp(strSymbol.c_str(), "< ") == 0)
		{
			m_dwSymbol = LessThan;
		}
		if (strcmp(strSymbol.c_str(), ">=") == 0)
		{
			m_dwSymbol = MoreThanOrEqual;
		}
		if (strcmp(strSymbol.c_str(), ">=") == 0)
		{
			m_dwSymbol = LessThanOrEqual;
		}
		break;
	}
	case 'l':// bl 查看断点列表命令
		//UserCommandBL(pCommand);
		break;
	default:
		printf("请输入正确的指令：\n");
		break;
	}
}

//************************************
// FullName:  CDbgEngine::UserCommandDisasm
// Returns:   void
// Parameter: CHAR * pCommand
//************************************
void CDbgEngine::UserCommandDisasm(CHAR* pCommand) {
	// 解析反汇编指令 u  地址 长度 长度可省略
	char seps[] = " ";
	char *token = NULL;
	char *next_token = NULL;
	// token = 'u'
	token = strtok_s(pCommand, seps, &next_token);
	// 反汇编地址
	// token = address(123456)
	token = strtok_s(NULL, seps, &next_token);
	if (token == nullptr) {
		printf("请输入正确的指令：\n");
		return;
	}
	DWORD dwAddress = strtol(token, NULL, 16);
	//sscanf_s(token, "%8x", &dwAddress);
	if (!dwAddress) {
		printf("请输入正确的指令：\n");
		return;
	}
	// 反汇编行数
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
	//2.3 一次反汇编1条,默认反汇编10条，可以自定义反汇编指令数目，也可以由输入命令指定
	printf("%-10s %-20s%-32s%s\n", "addr", "opcode", "asm", "comment");
	UINT uLen;
	for (DWORD i = 0; i < dwCount; i++) {
		// 反汇编
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
	// 1. 将调试程序的内存复制到本地
	DWORD  dwRetSize = 0;
	BYTE lpRemote_Buf[32] = {};
	ReadProcessMemory(hProcess, lpAddress, lpRemote_Buf, 32, &dwRetSize);
	// 2. 初始化反汇编引擎
	DISASM objDiasm;
	objDiasm.EIP = (UIntPtr)lpRemote_Buf; // 起始地址
	objDiasm.VirtualAddr = (UINT64)lpAddress;     // 虚拟内存地址（反汇编引擎用于计算地址）
	objDiasm.Archi = 0;                     // AI-X86
	objDiasm.Options = 0x000;                 // MASM
											  // 3. 反汇编代码
	UINT unLen = Disasm(&objDiasm);
	if (-1 == unLen) return unLen;
	// 4. 将机器码转码为字符串
	LPWSTR lpOPCode = pOPCode;
	PBYTE  lpBuffer = lpRemote_Buf;
	for (UINT i = 0; i < unLen; i++) {
		StringCbPrintf(lpOPCode++, 50, L"%X", *lpBuffer & 0xF0);
		StringCbPrintf(lpOPCode++, 50, L"%X", *lpBuffer & 0x0F);
		lpBuffer++;
	}
	// 6. 保存反汇编出的指令
	WCHAR szASM[50] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, objDiasm.CompleteInstr, -1, szASM, _countof(szASM));
	StringCchCopy(pASM, 50, szASM);
	return unLen;
}


//************************************
// Method:    GetStackInfo
// FullName:  CDbgEngine::GetStackInfo
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: DWORD dwThreadId
// Parameter: HANDLE hProcess
// Function:  查看堆栈信息
//************************************
BOOL CDbgEngine::GetStackInfo(DWORD dwThreadId, HANDLE hProcess)
{
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, NULL, dwThreadId);
	CONTEXT ct = { CONTEXT_CONTROL };
	GetThreadContext(hThread, &ct);
	BYTE buff[512];
	DWORD dwRead = 0;

	if (!ReadProcessMemory(hProcess, (LPVOID)ct.Esp, buff, 512, &dwRead))
	{
		return FALSE;
	}
	printf("%-10s %-10s\n", "Addr", "Value");
	for (int i = 0; i < 10; ++i){
		printf("%08X   %08X\n", (DWORD)ct.Esp+i*4,((DWORD*)buff)[i]);
	}
	return TRUE;
}

//************************************
// Method:    EnumModules
// FullName:  CDbgEngine::EnumModules
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: DWORD dwPid
// Function:  枚举被调试进程模块信息
//************************************
BOOL CDbgEngine::EnumModules(DWORD dwPid)
{
	m_vecModule.clear();
	//1.1打开进程
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
	if (NULL == hProcess) {
		return FALSE;
	}
	//1.2确定保存模块句柄的数组大小
	HMODULE hModules[0x1] = { 0 };//模块数组
	DWORD cbNeed = 0;//实际获取的大小
	EnumProcessModulesEx(/*枚举进程模块*/
		hProcess,//进程句柄
		hModules,//模块句柄数组
		sizeof(hModules),//模块句柄数组的大小
		&cbNeed,//实际需要的数组大小
		LIST_MODULES_ALL);//枚举模块的类型
	//1.3获取模块句柄表
	DWORD dwModCount = cbNeed / sizeof(HMODULE);//模块数量
	HMODULE* pModBuf = new HMODULE[dwModCount];//保存模块句柄的缓存
	EnumProcessModulesEx(/*枚举进程模块*/
		hProcess,//进程句柄
		pModBuf,//模块句柄数组
		dwModCount * sizeof(HMODULE),//模块句柄数组的大小
		&cbNeed,//实际需要的数组大小
		LIST_MODULES_ALL);//枚举模块的类型
	//2.枚举进程模块信息
	//char szModuleName[0x200] = {0};
	MODULEENTRY32 stcMod32 = { sizeof(MODULEENTRY32) };
	MODULEINFO stcModInfo;
	//3.分配vec空间
	for (UINT i = 0; i < dwModCount; i++) {
		GetModuleBaseName(hProcess, pModBuf[i], stcMod32.szModule, MAX_PATH);
		GetModuleFileNameEx(hProcess, pModBuf[i], stcMod32.szExePath, MAX_PATH);
		GetModuleInformation(hProcess, pModBuf[i], &stcModInfo, sizeof(MODULEINFO));
		stcMod32.hModule = pModBuf[i];
		stcMod32.modBaseAddr = (PBYTE)stcModInfo.lpBaseOfDll;
		stcMod32.modBaseSize = stcModInfo.SizeOfImage;
		stcMod32.th32ProcessID = dwPid;
		m_vecModule.push_back(stcMod32);
	}
	CloseHandle(hProcess);
	delete[] pModBuf;
	return TRUE;
}

//************************************
// Method:    PrintfModulesInfo
// FullName:  CDbgEngine::PrintfModulesInfo
// Access:    public 
// Returns:   VOID
// Qualifier:
// Function:  输出保存的模块信息
//************************************
VOID CDbgEngine::PrintfModulesInfo()
{
	printf("%-10s\t%-10s\t%-10s\t%-10s\n", "模块名称", "加载基址","模块大小","模块路径");
	for (MODULEENTRY32 &module : m_vecModule) {
		wprintf(L"%s\t%08X\t%d\t%s\n", module.szModule, module.modBaseAddr, module.modBaseSize, module.szExePath);
	}
}

VOID CDbgEngine::PrintfImportDll()
{
	printf("%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", "DLL名称", "INT RVA", "INT偏移", "IAT RVA", "IAT偏移");
	for (MY_IMPORT_DESCRIPTOR &importDescriptor : m_pLordPe->m_vecImportDescriptor)
	{
		wprintf(L"%s\t%08X\t%08X\t%08X\t%08X\n", importDescriptor.Name, importDescriptor.OriginalFirstThunk, 
		importDescriptor.OffsetOriginalFirstThunk, importDescriptor.FirstThunk, importDescriptor.OffsetFirstThunk);
	}
}

//************************************
// Method:    IsConditionBreakPoint
// FullName:  CDbgEngine::IsConditionBreakPoint
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: DWORD dwThreadId
// Function:  根据解析的条件表达式判断条件断点是否命中
//************************************
BOOL CDbgEngine::IsConditionBreakPoint(DWORD dwThreadId)
{
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, NULL, dwThreadId);
	CONTEXT ct = {};
	ct.ContextFlags = CONTEXT_ALL;// 指定要获取哪写寄存器的信息，很重要
	GetThreadContext(hThread, &ct);
	switch (m_dwLeft)
	{
	case EAX:
	{
		switch (m_dwSymbol)
		{
		case MoreThan:
			if (ct.Eax > m_dwRight)
				return TRUE;
			else
				return FALSE;
		case Equal:
			if (ct.Eax == m_dwRight)
				return TRUE;
			else
				return FALSE;
		case LessThan:
			if (ct.Eax < m_dwRight)
				return TRUE;
			else
				return FALSE;
		case MoreThanOrEqual:
			if (ct.Eax >= m_dwRight)
				return TRUE;
			else
				return FALSE;
		case LessThanOrEqual:
			if (ct.Eax <= m_dwRight)
				return TRUE;
			else
				return FALSE;
		}
	}
	case EBX:
	{
		switch (m_dwSymbol)
		{
		case MoreThan:
			if (ct.Ebx > m_dwRight)
				return TRUE;
			else
				return FALSE;
		case Equal:
			if (ct.Ebx = m_dwRight)
				return TRUE;
			else
				return FALSE;
		case LessThan:
			if (ct.Ebx < m_dwRight)
				return TRUE;
			else
				return FALSE;
		case MoreThanOrEqual:
			if (ct.Ebx >= m_dwRight)
				return TRUE;
			else
				return FALSE;
		case LessThanOrEqual:
			if (ct.Ebx <= m_dwRight)
				return TRUE;
			else
				return FALSE;
		}
	}
	case ECX:
	{
		switch (m_dwSymbol)
		{
		case MoreThan:
			if (ct.Ecx > m_dwRight)
				return TRUE;
			else
				return FALSE;
		case Equal:
			if (ct.Ecx = m_dwRight)
				return TRUE;
			else
				return FALSE;
		case LessThan:
			if (ct.Ecx < m_dwRight)
				return TRUE;
			else
				return FALSE;
		case MoreThanOrEqual:
			if (ct.Ecx >= m_dwRight)
				return TRUE;
			else
				return FALSE;
		case LessThanOrEqual:
			if (ct.Ecx <= m_dwRight)
				return TRUE;
			else
				return FALSE;
		}
	}
	case EDX:
	{
		switch (m_dwSymbol)
		{
		case MoreThan:
			if (ct.Edx > m_dwRight)
				return TRUE;
			else
				return FALSE;
		case Equal:
			if (ct.Edx = m_dwRight)
				return TRUE;
			else
				return FALSE;
		case LessThan:
			if (ct.Edx < m_dwRight)
				return TRUE;
			else
				return FALSE;
		case MoreThanOrEqual:
			if (ct.Edx >= m_dwRight)
				return TRUE;
			else
				return FALSE;
		case LessThanOrEqual:
			if (ct.Edx <= m_dwRight)
				return TRUE;
			else
				return FALSE;
		}
	}
	}
	CloseHandle(hThread);
}
