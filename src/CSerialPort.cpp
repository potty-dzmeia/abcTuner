#include "CSerialPort.h"
#include <Windows.h>
#include <assert.h>

CSerialPort::CSerialPort()
	: hCom(INVALID_HANDLE_VALUE)
{

}

CSerialPort::~CSerialPort()
{
	Close();
}

bool CSerialPort::Open(const char* port_name)
{
	assert(!IsOpen());

	Close();
	hCom = CreateFileA(port_name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hCom == INVALID_HANDLE_VALUE)
	{

		return FALSE; // Use GetLastError() to know the reason
	}

	DCB dcb;
	dcb.DCBlength = sizeof(dcb);
	GetCommState(hCom, &dcb);
	dcb.BaudRate = 9600;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = NOPARITY;
	dcb.fDsrSensitivity = 0;
	dcb.fOutxCtsFlow = 0;
	dcb.fOutxDsrFlow = 0;
	dcb.fInX = 0;
	dcb.fOutX = 0;
	dcb.fDtrControl = DTR_CONTROL_DISABLE; //DTR and RTS 0
	dcb.fRtsControl = RTS_CONTROL_DISABLE;

	SetCommState(hCom, &dcb);

	COMMTIMEOUTS touts;
	touts.ReadIntervalTimeout = 0;
	touts.ReadTotalTimeoutMultiplier = 0;
	touts.ReadTotalTimeoutConstant = 100;
	touts.WriteTotalTimeoutConstant = 10;
	touts.WriteTotalTimeoutMultiplier = 0;
	SetCommTimeouts(hCom, &touts);
	//SetCommMask(hCom, EV_CTS | EV_DSR | EV_RING | EV_RLSD);
	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);

	return TRUE;

}

void CSerialPort::Close()
{
	if (hCom != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hCom);
		hCom = INVALID_HANDLE_VALUE;
	}
}

bool CSerialPort::IsOpen()
{
	return (hCom != INVALID_HANDLE_VALUE);
}

unsigned long CSerialPort::Read(unsigned char *buff, unsigned long len)
{
	DWORD count(0);
	if (hCom != INVALID_HANDLE_VALUE)
	{
		ReadFile(hCom, buff, len, &count, NULL);
	}
	return count;
}

unsigned long CSerialPort::Write(const unsigned char *buff, unsigned long len)
{
	DWORD count(0);
	if (hCom != INVALID_HANDLE_VALUE)
	{
		WriteFile(hCom, buff, len, &count, NULL);
	}
	return count;
}
//
//BOOL CSerialPort::Get_CD_State()
//{
//	if (hCom != INVALID_HANDLE_VALUE)
//	{
//		DWORD ModemStat;
//		if (GetCommModemStatus(hCom, &ModemStat))
//		{
//			return (ModemStat & MS_RLSD_ON) > 0; //Not sure
//		}
//		else
//		{
//			return FALSE;
//		}
//	}
//	else
//	{
//		return FALSE;
//	}
//}
//
//BOOL CSerialPort::Get_CTS_State()
//{
//	if (hCom != INVALID_HANDLE_VALUE)
//	{
//		DWORD ModemStat;
//		if (GetCommModemStatus(hCom, &ModemStat))
//		{
//			return (ModemStat & MS_CTS_ON) > 0;
//		}
//		else
//		{
//			return FALSE;
//		}
//	}
//	else
//	{
//		return FALSE;
//	}
//}
//
//BOOL CSerialPort::Get_DSR_State()
//{
//	if (hCom != INVALID_HANDLE_VALUE)
//	{
//		DWORD ModemStat;
//		if (GetCommModemStatus(hCom, &ModemStat))
//		{
//			return (ModemStat & MS_DSR_ON) > 0;
//		}
//		else
//		{
//			return FALSE;
//		}
//	}
//	else
//	{
//		return FALSE;
//	}
//}
//
//BOOL CSerialPort::Get_RI_State()
//{
//	if (hCom != INVALID_HANDLE_VALUE)
//	{
//		DWORD ModemStat;
//		if (GetCommModemStatus(hCom, &ModemStat))
//		{
//			return (ModemStat & MS_RING_ON) > 0;
//		}
//		else
//		{
//			return FALSE;
//		}
//	}
//	else
//	{
//		return FALSE;
//	}
//}
//
//void CSerialPort::Set_DTR_State(BOOL state)
//{
//	if (hCom != INVALID_HANDLE_VALUE)
//	{
//		EscapeCommFunction(hCom, (state ? SETDTR : CLRDTR));
//	}
//}
//
//void CSerialPort::Set_RTS_State(BOOL state)
//{
//	if (hCom != INVALID_HANDLE_VALUE)
//	{
//		EscapeCommFunction(hCom, (state ? SETRTS : CLRRTS));
//	}
//}