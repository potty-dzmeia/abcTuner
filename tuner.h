#pragma once

#define C_MAX  (0xfff);
#define L_MAX  (0xfff);

struct relays {
	int c; // variable capacitor
	int l; // variable inductance
	bool c2; // enabled or disables capacitor
};

void command_set_relays(struct relays, char *data);
void command_set_tune(bool bTune, char *data);
void command_hi(char* data);
void command_request_adc(char *data);

int data_handler(const char *data);

int get_forward_v();
int get_backward_v();
int get_antenna_v();
int get_powersupply_v();
float get_swr();





