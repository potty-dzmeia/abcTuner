#pragma once

#include "myutils.h"

//----------------Configuration------------------
#define C_MAX  (4096)	// 12 bit value
#define L_MAX  (4096)	// 12 bit value
#define ANT_MAX (2)	// 3 antennas

//Mapping for variable Capacitor (raw to real values in [pF]) 
#define VALUE_MAPPING_FOR_C {29, 44, 46, 49, 56, 63,  88, 124, 204, 297,  537,  879, 1538}
//Mapping for variable Inductance (raw to real values [nH]) 
#define VALUE_MAPPING_FOR_L {280, 650, 670, 770, 790, 910, 1170, 1460, 2260, 3440, 6870, 12800, 23150}
//-----------------------------------------------


struct AtuRelays {
	U16 u16C; // variable capacitor C (0 to C_MAX)
	U16 u16L; // variable inductance L (0 to L_MAX)
	bool bC2; // If TRUE - C_ant is acitve (high R); If FALSE - C_tx is active (low R); 
	int u8Antenna; // Antenna selection (0 to ANT_MAX)
};
#define DEFAULT_AtuRelays { C_MAX, L_MAX, false, ANT_MAX }

struct AtuData {
	U16 u16Vf;
	U16 u16Vr;
	U16 u16Vant;
	U16 u16NotUsed;
};
#define DEFAULT_AtuData { 0, 0, 0, 0 }

bool atu_connect(const char *port_name);
void atu_disconnect();

void atu_set_relays(struct AtuRelays);
void atu_set_tune(bool *bTune);
bool atu_get_data(struct AtuData*);

unsigned int convert_C_pF(U16 u16C); // converts raw value to picoF (uses VALUE_MAPPINGS_FOR_C)
unsigned int convert_L_nH(U16 u16L); // converts raw value to nanoH (uses VALUE_MAPPINGS_FOR_L)





