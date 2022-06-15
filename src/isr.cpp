
#include <Arduino.h>
#include "isr.h"
#include "tyristorlib.h"
//For å velgja om ein skal ha pwm-signal eller firkantpuls på utgangane.
//Mest for testing og feilsøking, så er det lettare å setja utgangar høg istadenfor pulstog
#define SQUARE_WAVE 0

int start;
//int stopp;
//int flagg12;
//int flagg34;
static uint16_t period_count = 0;
uint8_t zc_watchdog_counter = 0;
uint16_t compA_counter = 0;


/*Set opp avbrotsfunksjon. Denne skal registrera eksternt avbrot for negativ flanke 
på pinne 2. */
void interruptSetup(){
  EICRA = (1 << ISC01) | (0 << ISC00);
  EIMSK = (1 << INT0);
}
/*Denne funksjonen køyrer for kvart avbrot. ZC_watchdog er lagt innf or å registrera
dersom det ikkje kjem inn firkantpulssignal*/
  ISR(INT0_vect){
  zc_watchdog_counter = 0;
  compA_counter = 0;
  if (start){               //Dersom start modus, viss ikkje skjer ingenting
      period_count = TCNT1; // Store counter value in order to calculate frequency
      TCNT1 = 0;            //Nullstiller timer, startar telja frå null
   
      OCR1A = get_comp_alpha();   //Set OCR1A til verdien som er rekna ut frå alfa frå potmeter/ESP
      OCR1B = get_comp_alpha() + get_comp_180(); //OCR1B er det same, ein halvperiode etter

    #if defined(SQUARE_WAVE) && (SQUARE_WAVE == 0) //Set enten pinne låg eller pulstog av. 
        T34pulseoff();
    #else
        PORTD &= ~(1 << 3); // Pin 3 low
    #endif
  }
  else {
  }
}

/*Denne funksjonen vil køyra kvar gong OCR1B = TCNT1. Det vil seia kvar gong ein får ...
*/

ISR(TIMER1_COMPB_vect){

#if defined(SQUARE_WAVE) && (SQUARE_WAVE == 0)          
    T34pulseon();//Set pinne3 høge eller pulstog på.

  #else 
    PORTD |= (1 << 3); // Pin 3 high
  #endif
}

ISR(TIMER1_COMPA_vect){ 
#if defined(SQUARE_WAVE) && (SQUARE_WAVE == 0)
  if (compA_counter > 0)/*is_pulse12_on()*/{   //Dersom compA_counter =< , så har ein positiv flanke og pulstoget skal vera av
      T12pulseoff();     
    }               
  else{                   //Her er negtiv flanke
    compA_counter++;      //For å kunna skru av ved positiv flanke
    T12pulseon();         //pulstoget skal på her ved negativ flanke
    // Oppdaterer OCR1A registeret
    // så timer kan skru av pulstoget
    // etter ein full syklus
    OCR1A = get_comp_180(); //20010;// - temp + TCNT1;
  }
  //Her skjer det same berre set utgangar høg og låg
#else
  if (compA_counter > 0){
    PORTB &= ~(1 << 3); // Pin 11 low
  }                                       
  else{
    compA_counter++;
    PORTB |= (1 << 3); // Pin 11 high
    OCR1A = get_comp_180(); //20010;// - temp + TCNT1;
  }
#endif
}



