#include "atu.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "myutils.h"
#include "CSerialPort.h"


#define RESPONSE_ACK_LEN	3 // Short status frame coming from the ATU is 3 bytes (ACK: [fe, fe, 1]; NACK: [fe, fe, 3])
#define RESPONSE_DATA_LEN	11 // The data frame coming from the ATU is 11 bytes [fe, fe, 2, a1, a2, a3, a4, a5, a6, a7, a8]
#define REQUEST_LEN		7 // Length of command which is send to ATU

#define BUFFER_SIZE		RESPONSE_DATA_LEN // general purpose buffer

#define BEGIN_FRAME		0x73
#define COMMAND_TUNE_OFF	0x01
#define COMMAND_TUNE_ON		0x02
#define COMMAND_GET_DATA	0x04
#define COMMAND_SET_RELAYS	0x08
#define COMMAND_HELLO		0x10

static struct AtuRelays stRelays = DEFAULT_AtuRelays;
static bool   bTune = false;
 
static U8 buffer[BUFFER_SIZE]; // Temp buffer

static bool compare(struct AtuRelays a, struct AtuRelays b);

static CSerialPort port;


/*
 * Parses the incoming data.
 * RETURN: number of bytes parsed
 */
bool atu_connect(const char *port_name)
{
	
	if (port.IsOpen())
		return true;

	if (!port.Open(port_name))
	{
		DEBUG_PRINT("Couldn't open port: %s", port_name);
		return false;
	}
	
	return true;
}


void atu_disconnect()
{
	if (port.IsOpen() == false)
		return; 

	port.Close();
	return;
}


void atu_set_relays(struct AtuRelays in)
{
	if (compare(stRelays, in))
		return; // nothing to do - values didn't change;

	stRelays = in;

	if (!port.IsOpen())
	{
		DEBUG_PRINT("Can't set relays. Not connected to Tuner.");
		return;
	}

	// REQUEST
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = BEGIN_FRAME;
	buffer[1] = COMMAND_SET_RELAYS; 

        /*BYTE     b0   b1   b2   b3   b4   b5   b6   b7
        ------------------------------------------------
        2         0     0    0    0    0     0    0    0
        3        C1    C2   C3   C4   C5    C6   C7   C8
        4        C9   C10  C11  C12   L1    L2   L3   L4
        5        L5    L6   L7   L8   L9   L10  L11  L12
        6       LoR   HiR    0   A1   A2    A3    0    0    */
	buffer[2] = 0x00;
	buffer[3] = (U8) (stRelays.u16C);
	buffer[4] = (U8) (((stRelays.u16C >> 0x08) & 0x0F) | ((stRelays.u16L << 4) & 0xF0));
	buffer[5] = (U8) (stRelays.u16L >> 4);

	if (stRelays.bC2)
		buffer[6] = 0x01; // C_ant is acitve (high R)
	else
		buffer[6] = 0x02; // C_tx is active (low R)

	// Set antenna
	switch (stRelays.u8Antenna)
	{
	case 0:
		buffer[6] = buffer[6] | 0x08;
		break;
	case 1:
		buffer[6] = buffer[6] | 0x10;
		break;
	case 2:
		buffer[6] = buffer[6] | 0x20;
		break;
	default:
		DEBUG_PRINT("Only 3 antennas currently supported");
		buffer[6] = buffer[6] | 0x08;
		break;
	}

	unsigned long count = 0;

	count = port.Write(buffer, REQUEST_LEN);
	if (count != REQUEST_LEN)
		DEBUG_PRINT("Tried to write %d byte(s) but only %d succeded!", REQUEST_LEN, count);
	
	// RESONSE
	count = port.Read(buffer, RESPONSE_ACK_LEN);
	if (count != RESPONSE_ACK_LEN)
		DEBUG_PRINT("No response to COMMAND_SET_RELAYS: %d bytes received", count);
	else
		DEBUG_PRINT("Set Relays command sent successfuly");
}


void atu_set_tune(bool *bTuneOn)
{
	if (bTune == *bTuneOn)
		return;

	bTune = *bTuneOn;

	if (!port.IsOpen())
	{
		DEBUG_PRINT("Can't set Tune mode. Not connected to Tuner.");
		bTune = *bTuneOn = false;
		return;
	}

	// REQUEST
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = BEGIN_FRAME;
	if (bTune)
		buffer[1] = COMMAND_TUNE_ON;
	else
		buffer[1] = COMMAND_TUNE_OFF;

	unsigned long count;

	count = port.Write(buffer, REQUEST_LEN);
	if (count != REQUEST_LEN)
		DEBUG_PRINT("Tried to write %d byte(s) but only %d succeded!", REQUEST_LEN, count);

	// RESPONSE
	count = port.Read(buffer, RESPONSE_ACK_LEN);
	if (count != RESPONSE_ACK_LEN)
	{
		bTune = *bTuneOn = false;
		DEBUG_PRINT("No response to COMMAND_TUNE_ON/COMMAND_TUNE_OFF: %d bytes received!", count);
	}
	else
	{
		DEBUG_PRINT("Tune command successfuly written.");
	}
			
}


bool atu_get_data(struct AtuData* out_data)
{
	if (!port.IsOpen())
	{
		//DEBUG_PRINT("Can't read Data. Not connected to Tuner.");
		return false;
	}

	// REQUEST
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = BEGIN_FRAME;
	buffer[1] = COMMAND_GET_DATA;

	unsigned long count;
	count = port.Write(buffer, REQUEST_LEN);
	if (count != sizeof(REQUEST_LEN))
		DEBUG_PRINT("Tried to write %d byte(s) but only %d succeded!", REQUEST_LEN, count);

	// RESPONSE
	count = port.Read(buffer, RESPONSE_DATA_LEN);
	if (count != RESPONSE_DATA_LEN)
	{
		DEBUG_PRINT("No response to COMMAND_GET_DATA: %d bytes received!", count);
		return false;
	}
	
	assert(buffer[2] == 2); // 3rd byte should be always 0x02
	out_data->u16Vf = buffer[4] << 8 | buffer[3];
	out_data->u16Vr = buffer[6] << 8 | buffer[5];
	out_data->u16Vant = buffer[8] << 8 | buffer[7];
	out_data->u16NotUsed = buffer[10] << 8 | buffer[9];

	// Calculate VSWR
	if (out_data->u16Vf == out_data->u16Vr)
		out_data->fSwr = SWR_MAX;
	else if(out_data->u16Vf < out_data->u16Vr)
		out_data->fSwr = SWR_MAX;
	else
		out_data->fSwr = (float) (out_data->u16Vf + out_data->u16Vr) / out_data->u16Vf - out_data->u16Vr;

	DEBUG_PRINT("COMMAND_GET_DATA successfully finished");
	return true;
}


unsigned int convert_C_pF(U16 u16C)
{
	static int mapping[] = VALUE_MAPPING_FOR_C;
	
	unsigned int result = mapping[0]; // the first entry is the stray capacitance

	for (int i = 1; i < sizeof(mapping); i++)
	{
		result += mapping[i] * (u16C & 0x01);
		u16C >>= 1;
	}

	return result;
}


unsigned int convert_L_nH(U16 u16L)
{
	static int mapping[] = VALUE_MAPPING_FOR_L;

	unsigned int result = mapping[0]; // the first entry is the stray inductnace

	for (int i = 1; i < sizeof(mapping); i++)
	{
		result += mapping[i] * (u16L & 0x01);
		u16L >>= 1;
	}

	return result;
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
