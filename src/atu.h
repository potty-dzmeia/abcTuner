#pragma once

#include "myutils.h"

#define C_MAX  (0xfff)
#define L_MAX  (0xfff)
#define ANT_MAX (2)

struct AtuRelays {
	U16 u16C; // variable capacitor
	U16 u16L; // variable inductance
	bool bC2; // enabled or disables capacitor
	int u8Antenna; // currently from 0-2
};
#define DEFAULT_AtuRelays { C_MAX, L_MAX, false, ANT_MAX }

struct AtuData {
	U16 u16Vf;
	U16 u16Vr;
	U16 u16Vant;
	U16 u16Vpsu;
};
#define DEFAULT_AtuData { C_MAX, L_MAX, false, ANT_MAX }

void atu_set_relays(struct AtuRelays);
void atu_set_tune(bool *bTune);
bool atu_get_data(struct AtuData*);

bool atu_connect(const char *port_name);
void atu_disconnect();

#ifdef UNIT_TEST
void parse_incoming_data(char *data, size_t len); //
void parse_incoming_data(char *data, size_t len); //
#endif //UNIT_TEST



