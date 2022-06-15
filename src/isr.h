#ifndef ISR_H
#define ISR_H

extern int start;
//extern int stopp;
//extern int flagg12;
//extern int flagg34;
extern uint8_t zc_watchdog_counter;
void ZC_ISR();
void interruptSetup();
const uint8_t ZC_pin = 2;

#endif /* ISR_H */