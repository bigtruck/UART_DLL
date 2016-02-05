// UART.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "UART.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO:  如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。  这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CUARTApp



BEGIN_MESSAGE_MAP(CUARTApp, CWinApp)
END_MESSAGE_MAP()

//IMPLEMENT_DYNCREATE(CUARTApp, CWinThread)

// CUARTApp 构造

CUARTApp::CUARTApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中

	memset(&osRead, 0, sizeof(osRead));
	memset(&osWrite, 0, sizeof(osWrite));
	memset(&ShareEvent, 0, sizeof(ShareEvent));
	COMFile = NULL;

	osRead.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (osRead.hEvent == NULL) {
		AfxMessageBox(_T("建立事件失败!"));
	}

	osWrite.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (osWrite.hEvent == NULL) {
		AfxMessageBox(_T("建立事件失败!"));
	}

	ShareEvent.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (ShareEvent.hEvent == NULL) {
		AfxMessageBox(_T("建立事件失败!"));
	}


}


// 唯一的一个 CUARTApp 对象

CUARTApp theApp;


// CUARTApp 初始化

BOOL CUARTApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}



int CUARTApp::ExitInstance()
{
	// TODO:    在此执行任意逐线程清理

	CloseHandle(osRead.hEvent);
	memset(&osRead, 0, sizeof(osRead));;
	CloseHandle(osWrite.hEvent);
	memset(&osWrite, 0, sizeof(osWrite));
	CloseHandle(ShareEvent.hEvent);
	memset(&ShareEvent, 0, sizeof(ShareEvent));



	return CWinThread::ExitInstance();
}

int CUARTApp::Run()
{
	while (1)
	{
		/*
		DWORD nResutl = WaitForMultipleObjectsEx(1, &osRead.hEvent, FALSE, 1, TRUE);
		if (1 == nResutl)
		{
		TRACE("\r\nwat\r\n");
		}
		*/

		DWORD dwEvtMask = 0;
		WaitCommEvent(COMFile, &dwEvtMask, &ShareEvent);//等待串口事件
		if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR)
		{
			//TRACE("\r\naa\r\n");
			DWORD  dwLen;
			dwLen = ReadData(m_GetBuff + m_GetIndex);
			m_GetIndex += (UINT32)dwLen;
			if (m_GetIndex >= MAX_GET_BUFF)
			{
				m_GetIndex -= MAX_GET_BUFF;
			}
			//TRACE("\r\nreaddata:%d\r\n",dwLen);
		}

	}
	//子线程结束
	CloseHandle(osRead.hEvent);
	CloseHandle(osWrite.hEvent);
	CloseHandle(ShareEvent.hEvent);

	return 0;
}

BOOL CUARTApp::OnIdle(LONG lCount)
{
	return 0;
}

BOOL CUARTApp::ConnectSerialPort(UINT port)
{
	DCB dcb;
	BOOL fRetVal;
	COMMTIMEOUTS CommTimeOuts;
	CString szCom;
	//szCom = L"\\\\.\\" + portName;
	szCom.Format(L"\\\\.\\COM%d", port);
	//COMFile = CreateFile(szCom.GetBuffer(50), GENERIC_READ | GENERIC_WRITE,//
	COMFile = CreateFile(szCom, GENERIC_READ | GENERIC_WRITE,//
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);
	if (INVALID_HANDLE_VALUE == COMFile) {
		return (FALSE);
	}

	SetupComm(COMFile, 6000, 6000);
	SetCommMask(/*COMFileTemp*/COMFile, EV_RXCHAR);
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 1000;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 2 * CBR_115200 / 115200;
	CommTimeOuts.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(/*COMFileTemp*/COMFile, &CommTimeOuts);

	dcb.DCBlength = sizeof(DCB);
	GetCommState(COMFile, &dcb);
	dcb.BaudRate = CBR_115200;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = NOPARITY;
	dcb.ByteSize = 8;
	dcb.fBinary = TRUE;
	dcb.fOutxDsrFlow = 0;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fOutxCtsFlow = 0;
	dcb.fRtsControl = RTS_CONTROL_ENABLE;
	dcb.fInX = dcb.fOutX = 1;
	dcb.XonChar = 0X11;
	dcb.XoffChar = 0X13;
	dcb.XonLim = 100;
	dcb.XoffLim = 100;
	dcb.fParity = TRUE;

	fRetVal = SetCommState(/*COMFileTemp*/COMFile, &dcb);

	if (!fRetVal)
	{
		return FALSE;
	}

	PurgeComm( /*COMFileTemp*/COMFile, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	EscapeCommFunction( /*COMFileTemp*/COMFile, SETDTR);


	return TRUE;
}

BOOL CUARTApp::CloseSerialPort(void)
{
	//禁止串行端口所有事件
	SetCommMask(COMFile, 0);
	//清除数据终端就绪信号
	EscapeCommFunction(COMFile, CLRDTR);
	//丢弃通信资源的输出或输入缓冲区字符并终止在通信资源上挂起的读、写操操作
	PurgeComm(COMFile, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	CloseHandle(COMFile);
	COMFile = NULL;

	return TRUE;
}

BOOL CUARTApp::WriteData(PBYTE pOutData, int nDataLen)
{
	if (pOutData == NULL || nDataLen == 0)
	{
		return FALSE;
	}

	DWORD dwLen;

	if (!WriteFile(COMFile, pOutData, nDataLen, &dwLen, &osWrite))
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(osWrite.hEvent, 0xFFFFFF))
			ResetEvent(osWrite.hEvent);
		else
			ResetEvent(osWrite.hEvent);

		//DWORD nError = GetLastError();
		//if (997 != nError) {//997异步没完成
		//	char chError[256]; memset(chError, 0, 256);
		//	int nBuffLen = 256;
		//	if (GetErrorString((PBYTE)chError, &nBuffLen, nError)) {
		//		AfxMessageBox(chError);
		//	}
		//}
	}
	else
	{
		TRACE("\r\nwrite error\r\n");
	}

	return TRUE;
}

DWORD CUARTApp::ReadData(PBYTE pInData)
{
	DWORD dwErrorFlags;
	COMSTAT comStat;
	DWORD dwLen, dwOutBytes;

	ClearCommError(COMFile, &dwErrorFlags, &comStat);
	dwLen = comStat.cbInQue;
	if (dwLen > MAX_GET_BUFF)
	{
		dwLen = MAX_GET_BUFF;
	}
	if (dwLen > 0)
	{
		ReadFile(COMFile, pInData, dwLen, &dwOutBytes, &osRead);
	}

	return dwLen;
}

UINT32 CUARTApp::UART_Count(void)
{
	UINT32 len = 0;

	if (m_GetIndex >= m_ReadIndex)
	{
		len = m_GetIndex - m_ReadIndex;
	}
	else
	{
		len = MAX_GET_BUFF - m_ReadIndex + m_GetIndex;
	}
	return len;
}

UCHAR CUARTApp::UART_GetData(void)
{
	UCHAR	ch;

	ch = m_GetBuff[m_ReadIndex];
	if (++m_ReadIndex >= MAX_GET_BUFF)
	{
		m_ReadIndex = 0;
	}
	return ch;
}

void CUARTApp::ClearBuffer(void)
{
	m_GetIndex = 0;
	m_ReadIndex = 0;
}

UINT16 CUARTApp::ScanSerialPort(UCHAR * numBuff)
{
#define MAX_KEY_LENGTH   255
#define MAX_VALUE_NAME  16383
	HKEY hTestKey;
	int nIndex = 0;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hTestKey))
	{
		TCHAR   achClass[MAX_PATH] = TEXT(""); // buffer for class name
		DWORD   cchClassName = MAX_PATH; // size of class string
		DWORD   cSubKeys = 0;                  // number of subkeys
		DWORD   cbMaxSubKey;             // longest subkey size
		DWORD   cchMaxClass;             // longest class string
		DWORD   cValues;             // number of values for key
		DWORD   cchMaxValue;         // longest value name
		DWORD   cbMaxValueData;      // longest value data
		DWORD   cbSecurityDescriptor; // size of security descriptor
		FILETIME ftLastWriteTime;     // last write time

		DWORD i, retCode;
		//TCHAR achValue[MAX_VALUE_NAME];
		CHAR achValue[MAX_VALUE_NAME];
		DWORD cchValue = MAX_VALUE_NAME;
		LONG lResult;

		// Get the class name and the value count.
		retCode = RegQueryInfoKey(
			hTestKey,                   // key handle
			achClass,               // buffer for class name
			&cchClassName,          // size of class string
			NULL,                   // reserved
			&cSubKeys,              // number of subkeys
			&cbMaxSubKey,           // longest subkey size
			&cchMaxClass,           // longest class string
			&cValues,               // number of values for this key
			&cchMaxValue,           // longest value name
			&cbMaxValueData,        // longest value data
			&cbSecurityDescriptor,  // security descriptor
			&ftLastWriteTime);      // last write time
		if (cValues > 0)
		{

			for (i = 0; i<cValues; i++)
			{
				cchValue = MAX_VALUE_NAME;
				achValue[0] = '\0';
				lResult = RegEnumValueA(hTestKey, i, achValue, &cchValue, NULL, NULL, NULL, NULL);
				if (ERROR_SUCCESS == lResult)
				{
					BYTE strDSName[50];
					//memset(strDSName, 0, 100);
					strDSName[0] = '\n';
					DWORD  nBuffLen = 50;
					//if (ERROR_SUCCESS == RegQueryValueEx(hTestKey, (LPCTSTR)achValue, NULL,&nValueType, strDSName, &nBuffLen))
					if (ERROR_SUCCESS == RegQueryValueExA(hTestKey, achValue, NULL, NULL, strDSName, &nBuffLen))
					{
						//						UCHAR num;
						numBuff[nIndex++] = atoi((char*)(strDSName + 3));
						//CString str(strDSName);
						//combobox->AddString(str);
					}
				}
				else
				{
					TRACE("error\r\n");
				}
			}
		}
		else
		{
			//AfxMessageBox(_T("PC机没有COM口....."));
		}
	}
	RegCloseKey(hTestKey);
	return nIndex;
}

//BEGIN_MESSAGE_MAP(CUARTApp, CWinThread)
//END_MESSAGE_MAP()

