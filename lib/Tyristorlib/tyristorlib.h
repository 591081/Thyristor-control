#ifndef tyristorlib_h
#define tyristorlib_h


void T12pulseon();
void timer1off();
void T12pulseoff();
void T34pulseon();
void T34pulseoff();
void timer1setup();
void timer2setup();
void timer1on();

/**
 * @brief 
 * 
 * @param alpha 0 - 180
 * @param grid_freq 
 */
void timer1setalpha(uint16_t alpha,uint8_t grid_freq);

uint16_t get_comp_alpha();
uint16_t get_comp_180();
bool is_pulse12_on();
bool is_pulse34_on();

#endif /* tyristorlib.h*/