#pragma once

class CSerialPort
{
public:
	/*void Set_RTS_State(BOOL state);
	void Set_DTR_State(BOOL state);
	BOOL Get_RI_State();
	BOOL Get_DSR_State();
	BOOL Get_CTS_State();
	BOOL Get_CD_State();*/
	
	unsigned long Write(const unsigned char *buffer, unsigned long len);
	unsigned long Read(unsigned char  *buffer, unsigned long len);
	bool IsOpen();
	void Close();
	bool Open(const char *port_name);
	CSerialPort();
	virtual ~CSerialPort();

protected:
	void* hCom;
};