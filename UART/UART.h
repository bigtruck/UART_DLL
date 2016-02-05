// UART.h : UART DLL 的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

#include <afxmt.h>

#define MAX_GET_BUFF	(1024*10)
#define MAX_SEND_BUFF	(1024*10)


// CUARTApp
// 有关此类实现的信息，请参阅 UART.cpp
//

class CUARTApp : public CWinApp , public CWinThread
//class CUARTApp : public CWinThread
{
public:
	CUARTApp();

// 重写
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
