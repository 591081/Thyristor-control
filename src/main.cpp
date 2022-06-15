#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "tyristorlib.h"
#include "isr.h"

////////////////////////////////////////////////
/// settings //

const uint8_t grid_freq_setting_hz = 50;


////////////////////////////////////////////////

int mode;
char c;
uint8_t T12_pin = 11;  //Pin11 som gjev ut pulstogsignal til eitt av tyristorpara
uint8_t T34_pin = 3;   //Pin3 som gjev ut pulstogsignal til det andre tyritorparet
uint8_t test_pin = 10;
void lc();
void lcoff();
uint32_t pot_pin = A0;
uint32_t potmeter_adc_raw;
uint32_t alpha_degrees;
uint32_t alpha_percentage;
uint32_t alpha_from_pot;
uint32_t alpha_from_ESP;



//Sett opp symbol for alpha, som skal visast i display
uint8_t alpha[] = {0x00, 0x09, 0x15, 0x12, 0x12, 0x0D, 0x00, 0x00};


//Oppsett av LCD, vel ledige pinnar 
const uint8_t rs = 4, en = 5, d4 = 6, d5 = 7, d6 = 8, d7 = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte slaveAdr = 127;
void slaverecievefrommaster(int howMany);

void setup() {
  Serial.begin(9600);
  pinMode(test_pin, OUTPUT);
  pinMode(T12_pin, OUTPUT);
  pinMode(T34_pin, OUTPUT);
  pinMode(pot_pin, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  timer2setup();
  timer1setup();
  interruptSetup();
  lcd.begin(16, 2);
  start = 0;            //Kvar gong programmet vert lasta opp er det i stopp-modus
  pinMode(10, OUTPUT);
// For I2C:
  Wire.begin(127);
  Wire.onReceive(slaverecievefrommaster);
  c = 'L';
  pinMode(4, OUTPUT);
}

/*Funksjon som styrer lcd. Hentar alfa verdiar som skal visast
og kva modus programmet er i. Funksjonen vert kalla kvart 200 ms i loop.*/
void lc(uint16_t alpha_degrees,uint16_t alpha_percentage){
  lcd.clear();
  lcd.createChar(1, alpha);
  lcd.setCursor(0,0);
  lcd.write(byte(1));
  lcd.print(" = "); 
  lcd.print(alpha_degrees);
  lcd.print("\xDF");
  lcd.setCursor(0,1);
  lcd.print(alpha_percentage);
  lcd.print(" %");
  lcd.setCursor(12,0);
  if (start){ 
    lcd.print("ON");}
  else{
    lcd.print("OFF");
  }
    lcd.setCursor(10, 1);
  if (mode){
    lcd.print("Remote");
  }
  else{
    lcd.print("Local");
  }
}

struct {
  uint8_t pressed;
  uint8_t previous;
  uint8_t changed;
} button_state;

/*Funksjon for å hindra debounce av knappar. Vert kalla kvart 10ms. Set opp ein 8bit currentState
som les inn verdien til analoge innganagr kor knappane er tilkopla.  */

void SamplePushbuttons (){
  
  uint8_t currentState =  (!digitalRead(A4) << 1)| digitalRead(A3) | (digitalRead(A5) << 2);

  button_state.changed = button_state.pressed;

  button_state.pressed |= (button_state.previous & currentState);
  button_state.pressed &= (button_state.previous | currentState);

  button_state.changed ^= button_state.pressed;
  button_state.previous = currentState;
  
}

//Funksjon som les inn data sendt frå ESP32 og lagrar desse
void slaverecievefrommaster(int howMany){
   while (Wire.available()) { 
     c = Wire.read();        
     alpha_from_ESP = Wire.read();
     Serial.println(c);
     delay(500);
 }
}

void loop() {
  uint32_t currentTimeStamp_ms = millis();
  static uint32_t prevTimeStamp_ms = 0;
  const uint32_t interval_ms = 10;
  const uint32_t intervallc_ms = 200;
  static uint32_t prevTimeStamplc_ms = 0;
 
if ((currentTimeStamp_ms - prevTimeStamp_ms) >= interval_ms){
  SamplePushbuttons();
  //Sjekkar om startknapp er trykt
  if ((button_state.changed & (1<<0)) != 0){
    if ((button_state.pressed & (1<<0)) != 0){
    }
    else {
      start= 1; // Set startflagg til ein slik at resten av koden kan køyra frå avbrotsfunksjon
      timer1on();
      }    
  }
//Sjekkar om stoppknapp er trykt
 if ((button_state.changed & (1<<1)) != 0){
    if ((button_state.pressed & (1<<1)) != 0){
      start = 0; //I stoppmodus skjer ingenting og alt er skrudd av 
      timer1off();
      T12pulseoff();
      T34pulseoff();
    }
    else {
    }
  }
    zc_watchdog_counter++; //sjekkar om firkanpulssignal manglar, i såfall send feilmelding
    if(zc_watchdog_counter > 15){
      Serial.println("Zero crossing signal is missing!");
    }
  prevTimeStamp_ms = currentTimeStamp_ms;
}
if ((currentTimeStamp_ms - prevTimeStamplc_ms) >= intervallc_ms){
   potmeter_adc_raw = analogRead(pot_pin); //Les verdi frå potmeter, 0-1023
   //sjekkar kva modus programmet er i og kva alfaverdi det skal bruka
   alpha_from_pot = ((potmeter_adc_raw*180)/1023);
   //alpha_from_ESP = x;
  if (c == 'R'){
    alpha_degrees = alpha_from_ESP;
    mode = 1;
  }
  else if (c == 'L'){
    alpha_degrees = alpha_from_pot;
    mode = 0;
  }
alpha_percentage = 100 - ((alpha_degrees*100)/180);//Reknar ut kor mange prosent tennvinkelen er
timer1setalpha(alpha_degrees, grid_freq_setting_hz);//Gjev verdiar til funksjon for omrekning til OCRxy verdiar
lc(alpha_degrees, alpha_percentage);//Same verdiane til lc funksjonen
prevTimeStamplc_ms = currentTimeStamp_ms;
 }
}
