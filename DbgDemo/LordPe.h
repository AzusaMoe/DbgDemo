#pragma once
#include "windows.h"


class CLordPe
{
public:
	CLordPe();
	~CLordPe();
	BOOL GetDosHead(LPCTSTR filePath);
	DWORD GetOep();
	LPCVOID getLoadAddress(DWORD dwProcessId);
private:
	BYTE* m_pBuf;//�����ͷ�����Ŀռ�
	PIMAGE_DOS_HEADER m_pDosHdr;//DOSͷ��ַ
};