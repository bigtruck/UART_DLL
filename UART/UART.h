// UART.h : UART DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

#include <afxmt.h>

#define MAX_GET_BUFF	(1024*10)
#define MAX_SEND_BUFF	(1024*10)


// CUARTApp
// �йش���ʵ�ֵ���Ϣ������� UART.cpp
//

class CUARTApp : public CWinApp , public CWinThread
//class CUARTApp : public CWinThread
{
public:
	CUARTApp();

// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()

public:
	CSemaphore *m_pSemaphore;
	OVERLAPPED osRead;
	OVERLAPPED osWrite;
	OVERLAPPED ShareEvent;

	UCHAR m_GetBuff[MAX_GET_BUFF];
	UINT32 m_GetIndex;
	UINT32 m_ReadIndex;
	HANDLE COMFile;

public:
	virtual int Run();
	virtual BOOL OnIdle(LONG lCount);
	BOOL ConnectSerialPort(UINT port);
	BOOL CloseSerialPort(void);
	BOOL WriteData(PBYTE pOutData, int nDataLen);
	DWORD ReadData(PBYTE pInData);
	UINT32 UART_Count(void);
	UCHAR UART_GetData(void);
	void ClearBuffer(void);
	UINT16 ScanSerialPort(UCHAR *numBuff);

};
