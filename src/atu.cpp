#include "atu.h"
#include <assert.h>
#include <windows.h>
#include <stdio.h>
#include "myutils.h"


#define STATUS_FRAME_LEN  3  // Short status frame coming from the ATU is 3 bytes (PosCfm: [fe, fe, 1] or NegCfm: [fe, fe, 3])
#define DATA_FRAME_LEN  11  // The long data frame coming from the ATU is 11 bytes [fe, fe, 2, a1, a2, a3, a4, a5, a6, a7, a8]
#define COMMAND_FRAME_LEN 7 // Length of command which is send to ATU

static struct AtuRelays stRelays = DEFAULT_AtuRelays;
static bool   bTune = false;

static HANDLE hCom = INVALID_HANDLE_VALUE;
 
static U8 buffer[DATA_FRAME_LEN]; // Temp buffer

static bool is_connected();
static bool compare(struct AtuRelays a, struct AtuRelays b);

/*
 * Parses the incoming data.
 * RETURN: number of bytes parsed
 */
bool atu_connect(const char *port_name)
{

	COMMTIMEOUTS timeouts;
	DCB dcbCom;
	if (hCom != INVALID_HANDLE_VALUE)
		return true;

	hCom = CreateFileA(port_name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hCom == INVALID_HANDLE_VALUE)
	{
		DEBUG_PRINT("Couldn't open port: %s", port_name)
		return false;
	}
		
	if (!GetCommTimeouts(hCom, &timeouts))
	{
		atu_disconnect();
		DEBUG_PRINT("Couldn't get Comm timeouts: %s", port_name)
		return false;
	}
		
	timeouts.ReadIntervalTimeout = 0; // MAXDWORD
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;
	if (!SetCommTimeouts(hCom, &timeouts))
	{
		atu_disconnect();
		DEBUG_PRINT("Couldn't set Comm Timeouts")
		return false;
	}

	if (!GetCommState(hCom, &dcbCom))
	{
		atu_disconnect();
		DEBUG_PRINT("Couldn't get Comm state")
		return false;
	}

	//baude rate and others
	dcbCom.DCBlength = sizeof dcbCom;
	dcbCom.BaudRate = 3600;
	dcbCom.ByteSize = 8;
	dcbCom.StopBits = ONESTOPBIT;
	dcbCom.Parity = NOPARITY;
	dcbCom.fRtsControl = 1;
	if (!SetCommState(hCom, &dcbCom))
	{
		atu_disconnect();
		DEBUG_PRINT("Couldn't set Comm state: %s", port_name)
		return false;
	}
	
	return true;
}


void atu_disconnect()
{
	if (hCom == INVALID_HANDLE_VALUE)
		return;

	CloseHandle(hCom);
	hCom = INVALID_HANDLE_VALUE;
	return;
}


void atu_set_relays(struct AtuRelays in)
{
	if (compare(stRelays, in))
		return; // nothing to do - values didn't change;

	stRelays = in;

	if (!is_connected())
	{
		DEBUG_PRINT("Can't set relays. Not connected to Tuner.")
		return;
	}

	//   73   |   mode           |    B0     |    B1   |   B2    |  B3     |  B4
	//           08(Load relays)   always 0     
	U8 serial_command[7];
	memset(serial_command, 0, sizeof(serial_command));

	serial_command[0] = 0x73;   // Magic byte
	serial_command[1] = 0x08;   // Load Relays
	serial_command[2] = 0x00;   // Always 0
	serial_command[3] = (U8) (stRelays.u16C); // LO(c)
	serial_command[4] = (U8)((stRelays.u16C >> 0x08) | (((~stRelays.u16L) & 0x0F) << 4));     // HI(c) + LO(l)     Note:  bits controlling L must be negated
	serial_command[5] = (U8) (~(stRelays.u16L >> 4));     // HI(l)

	// Set C2 active
	if (stRelays.bC2)
		serial_command[6] = 0x01; // C2 active
	else
		serial_command[6] = 0x02; // C1 active

	// Set antenna
	switch (stRelays.u8Antenna)
	{
	case 0:
		serial_command[6] = serial_command[6] | 0x08;
		break;
	case 1:
		serial_command[6] = serial_command[6] | 0x10;
		break;
	case 2:
		serial_command[6] = serial_command[6] | 0x20;
		break;
	default:
		DEBUG_PRINT("Only 3 antennas currently supported")
		break;
	}

	// Low L:    M1 = 0  CM3 = 1 ;       Hi L:  M1 = 1    CM3 = 0       low L  if L <512;  High L  if L >= 512
	if (stRelays.u16L<512)
		serial_command[6] = serial_command[6] | 0x40;
	else

		serial_command[6] = serial_command[6] | 0x04;

	unsigned long count;
	WriteFile(hCom, &serial_command, sizeof(serial_command), &count, 0);

	if (count != sizeof(serial_command))
		DEBUG_PRINT("Tried to write %d byte(s) but only %d succeded!", sizeof(serial_command), count)

	DEBUG_PRINT("Relays command successfuly written!")

	
	// W8 for Response
	ReadFile(hCom, &count, 1, &count, NULL);//cheta i ako FE: All is OK
}


void atu_set_tune(bool *bTuneOn)
{
	if (bTune == *bTuneOn)
		return;

	bTune = *bTuneOn;

	if (!is_connected())
	{
		DEBUG_PRINT("Can't set Tune mode. Not connected to Tuner.")
		bTune = *bTuneOn = false;
		return;
	}

	//   73   |   mode           |    B0     |    B1   |   B2    |  B3     |  B4
	//                    always 0     
	U8 serial_command[7];
	memset(serial_command, 0, sizeof(serial_command));

	serial_command[0] = 0x73; 
	if (bTune)
		serial_command[1] = 0x02;   // Command 
	else
		serial_command[1] = 0x01;

	unsigned long bytes_written;
	WriteFile(hCom, &serial_command, sizeof(serial_command), &bytes_written, 0);

	if (bytes_written != sizeof(serial_command))
		DEBUG_PRINT("Tried to write %d byte(s) but only %d succeded!", sizeof(serial_command), bytes_written)

	DEBUG_PRINT("Tune command successfuly written.")
	for (int i=0; i < sizeof(serial_command); i++)
	{
		printf("0x%x, ",serial_command[i]);
	}
}

bool atu_get_data(struct TunerData* inDatas)
{
	if (!is_connected())
	{
		DEBUG_PRINT("Can't read Data. Not connected to Tuner.")
		return false;
	}

	// REQUEST
	U8 serial_command[7];
	memset(serial_command, 0, sizeof(serial_command));

	serial_command[0] = 0x73;   // Magic byte
	serial_command[1] = 0x04;   // Command - 00000100  SEND-ADC Send back ADC data 8 bytes

	unsigned long bytes_written;
	WriteFile(hCom, &serial_command, sizeof(serial_command), &bytes_written, 0);

	if (bytes_written != sizeof(serial_command))
		DEBUG_PRINT("Tried to write %d byte(s) but only %d succeded!", sizeof(serial_command), bytes_written)

	// RESPONSE
	

	

}


static bool compare(struct AtuRelays a, struct AtuRelays b)
{
	if (a.bC2 == b.bC2 &&
		a.u16C == b.u16C &&
		a.u16L == b.u16L &&
		a.u8Antenna == b.u8Antenna)
		return true;
	else
		return false;
}


static bool is_connected()
{
	return hCom != INVALID_HANDLE_VALUE;
}