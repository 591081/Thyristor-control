/**
 * @file tyristorlib.cpp
 * @author Eli Laupsa
 * @brief 
 * @version 0.1
 * @date 2022-04-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Arduino.h>
#include "tyristorlib.h"

uint16_t comp_180 = 0;     //maksimum verdi av alfa
uint16_t comp_alpha = 0;   //Faktisk verdi av alfa

/*Funksjonar som skrur av og på timer1. */
void timer1on(){
  TCNT1 = 0;
  TIMSK1 |= (1<<OCIE1A) | (1<<OCIE1B);
}

void timer1off(){
  TCNT1 = 0;
  TIMSK1 &= ~((1<<OCIE1A)| (1<<OCIE1B));
}

/*Funksjonar som skrur av og på pulstog på ulike pinnar. T12 er pinne 11 og T34 er pinne 3. I tilegg
funksjonar for å sjekka om pulstoga er på eller ikkje. Dei gjev verdi true for på og 
false for av. Desse verdiane blir brukt i compA vektor*/
void T12pulseon() {
  TCNT2 = 0;
  TCCR2A |=  (1<<COM2A0) | (1<<COM2A1);
  TCCR2B |= ((0<<CS22) | (0<<CS21)|(1<<CS20));
}

void T12pulseoff(){
  TCCR2A &= ~( (1<<COM2A0) | (1<<COM2A1));
  TCCR2B &= ~((1<<CS22) | (1<<CS21)|(1<<CS20));
}

bool is_pulse12_on(){
  if (((TCCR2A >> COM2A0) & 0x01) && ((TCCR2A >> COM2A1) & 0x01)){
    return true;
  }
  else{
    return false;
  }
}

bool is_pulse34_on(){
  if (((TCCR2A >> COM2B1) & 0x01) && ((TCCR2A >> COM2B0) & 0x01)){
    return true;
  }
  else{
    return false;
  }
}

void T34pulseon(){
  TCNT2= 0;
  TCCR2A |= (1<<COM2B1 ) | (1<<COM2B0);
  TCCR2B |= ((0<<CS22) | (0<<CS21)|(1<<CS20));
}

void T34pulseoff(){
 TCCR2A &= ~((1<<COM2B1 ) | (1<<COM2B0));
 TCCR2B &= ~((1<<CS22) | (1<<CS21)|(1<<CS20));
}

/*Innstilling/Opsett for timerane. Timer2 skal gje ut pulstog, pwm signal. 
OCR1A og B bestemmer arbeidssyklus. Timer1 skal berre telja vanleg oppover.*/
void timer2setup(){
  TCCR2A = _BV(COM2A0)|/*_BV(COM2A1)|*/_BV(WGM22) |/* _BV(WGM21) |*/ _BV(WGM20);
  TCCR2B = _BV(CS22)|  _BV(CS21) | _BV(CS20);
  OCR2A = 130; //pin 11 
  OCR2B = 130; //pin 3  
}

void timer1setup(){   
  TCCR1A =0;   
  TCCR1B = (0 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);               
}


// Alpha som vert henta her, er frå 0 til 180 grader.  

void timer1setalpha(uint16_t alpha, uint8_t grid_freq){
 
  comp_180 = (16000000/(8*2*grid_freq));  //Reknar ut alfa for 180 grader etter kva frekvens som er vald
  uint32_t x = ((180*1000000)/comp_180);  //

  if (alpha > 178){ //For å unngå glitching på pulstog, vert ustabilt på 180,blir aldri < 178
    alpha = 178;
    }

  comp_alpha = ((alpha*1000000)/x); //Faktisk alfaverdi frå potmeter/ESP rekna om til OCRxy-verdi til timer

  if(0 == comp_alpha){  //Alfa kan heller ikkje verta 0,då startar aldri timer og det vert ustabilt
    comp_alpha = 1;
  }

  if (comp_alpha > (comp_180 - 20)){ //
    comp_alpha -= 20;
  }

}

//Funksjonar for å henta oppdaterte alfaverdiar til mainprogrammet. OCRxy verdiar. 

uint16_t get_comp_alpha(){
  return comp_alpha;
}

uint16_t get_comp_180(){
  return comp_180;
}