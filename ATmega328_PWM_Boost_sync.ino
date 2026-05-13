/*
   ATmega328_PWM_Boost_sync.ino

   Created : 20 June 2023
   Author  : yopie DIY

   PWM clock = fXTAL/freqPWM = 16,000,000/10,000 = 1,600 clock. 
   WGM mode 8 is used, so ICR1 = 1,600/2 = 800 clk
*/

const byte vfbPin = A0;
const byte potPin = A3;
const byte pwmOutA = 9;               // PB1 OC1A 
const byte pwmOutB = 10;              // PB2 OC1B

const long fXTAL = 16000000;          // crystal freq.
const int fPWM = 31250;               // fMax 31250 Hz
const int icr = ((fXTAL / fPWM) / 2);
const int deadTime = 5;               // deadTime = 62.5nS * 5 = 312.5nS

volatile int dutyOn = icr * 0.1;
int analogReadTolerance = 4;          // adjust as needed (compensation for noise and component tolerances)
int voltageReff = 307;                // preset output @ 18V  (1.5V @ vfbPin)

void initPort() {
  pinMode(pwmOutA, OUTPUT);
  pinMode(pwmOutB, OUTPUT);
}

void initTimer() {
//  TCCR1A = 0b00110000;    // non-Sync Boost converter configuration (OC1A disable OC1B enable)
  TCCR1A = 0b10110000;     // Sync Boost ( OC1A & OC1B enable)
  TCCR1B = 0b00010001;    //WGM1 3:2 = 10 (Waveform Generation Mode 8), CS1 2:0 = 001 (no prescale)
  TIMSK1 = 0b00000001;    // xxxxxxx1 TOV1, interrupt when counter overfow. 
  ICR1 = icr;             // Counter TOP value.
  OCR1B = icr - dutyOn;               // lowside switch (sync & non sync operation)
  OCR1A = icr - dutyOn - deadTime;    // highside switch (sync only)
}

void setup() {
  initPort();
  initTimer();
  Serial.begin(115200);
}

void loop() {

//  long voltageReff; 
//  voltageReff = 311;

  int voltageFeedBack = analogRead(vfbPin);
//  if (voltageFeedBack + analogReadTolerance > voltageReff) dutyOn--;   // Decrease Duty Cycle
//  if (voltageFeedBack - analogReadTolerance < voltageReff) dutyOn++;   // Increase Duty Cycle
  if (voltageFeedBack > (voltageReff + analogReadTolerance)) {
    dutyOn--;
  } else if (voltageFeedBack < (voltageReff - analogReadTolerance)) {
    dutyOn++;
  }
//  delay(1);
}

ISR(TIMER1_OVF_vect) {
//  int temp = icr - dutyOn;
  OCR1A = icr - dutyOn - deadTime;
  OCR1B = icr - dutyOn;
}
