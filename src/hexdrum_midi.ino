//////
/////         //    // /////// //   //     ////     //////    //  //  ///   //
/////        //    // /        // //      //  //   //  ///   //  //  // / ///
/////       //////// ////      ///       //   //  // ///    //  //  //  / //
/////      //    // /        //  //     //   //  //   //   //  //  //    //
/////     //    // //////  //    //    // ///   //    //   ////   //    //
/////
/////     HEX DRUM is a midi-capable expanded port of the "big button" by 
//////       look mum no computer
/////     modified by extralife in 2020
/////        extralifedisco@gmail.com
/////     no commercial use permitted
/////     re-use and modification permitted with attribution
/////     original "dont be a pleb" license included below
/////

//KOSMO BIG BUTTON 2016 SAM BATTLE
//email address is computer@lookmumnocomputer.com
//look mum no computer
//lookmumnocomputer@gmail.com
//www.facebook.com/lookmumnocomputer
//ITS AN ABSOLUTE MESS BUT IT WORKS.... SORT OF....
//YOU NEED QUITE A CLEAN CLOCK TRIGGER SIGNAL. QUITE A SHORT PULSE!
//To make it work better add this circuit to the clock input :-
//https://www.cgs.synth.net/modules/cgs24_gatetotrigger.html

//the premise of this is a simple performance sequencer.
//it is used in synth bike mk2 to sequencer the drums.
//i figured whats the point in not sharing it!!!
//dont be a pleb and steal it for a product or some shit. Build it and
//enjoy it as a musical instrument :)

// Clock in is pin 2
// Clear Button is pin 4 .... this clears the whole loop sequence
// Button Delete is pin 7 This deletes the step your on
// Bank select Pin 3 .... each channel has 2 banks of patterns, so you can record 2 alternative patterns and go between with this button
// BIG BUTTON is pin 19 (A5), this is the performance button!
// Reset in is pin 6 you can add a button or a jack!
// FILL BUTTON pin 5, push this and it will continuously play the channel your on, doesnt record, push it and twist the select knob to make a fill!
// STEP LENGTH analog pin 1 (A1)
// Channel select Analog pin (A0)
// SHIFT KNOB Analog pin (A2)
// CLOCK OUT (A3) (digital)
// LED (big button LED) pin 20 (A6)



#include <Arduino.h>
#include <SoftwareSerial.h>

const int OUT1 = 8;
const int OUT2 = 9;
const int OUT3 = 10;
const int OUT4 = 11;
const int OUT5 = 12;
const int OUT6 = 13;
const int outputs[6] = { OUT1, OUT2, OUT3, OUT4, OUT5, OUT6 };
const int BankLED = A6;//17
const int ClockOut = A3;

const int ClockIn      = 2;
const int BigButton    = A5;
const int ButtonBank   = 3;
const int ButtonClear  = 4;         //reset button for the moment
const int FillButton   = 5;
const int ResetButton  = 6;
const int ButtonDelete = 7;

#define TOLERANCE 0
#define CLOCK_PULSE_LENGTH 10 //milliseconds

//loop operators
int i; 
int ii;

//Clock Reset Keepers
int ClockKeep = 0;
int looper = 0;
//int buttonPushCounter = 0;   // counter for the number of button presses

//Various button states
int BigButtonState = 0;
int prevBigButtonState = 0;
int DeleteButtonState = 0;
int prevDeleteButtonState = 0;
int ClearButtonState = 0;
int ResetButtonState = 0;
int prevResetButtonState = 0;
int FillButtonState = 0;

//Bank Button Latching states
long time = 0;
long debounce = 150;
int BankButtonState;
int BankButtonPrevState;
int bankChannelStates[6] = { 0, 0, 0, 0, 0, 0 };

//SHIFT KNOB
int shiftKnobVal = 0; //analog value
int shiftOldKnobVal = 0; //for detecting changes
int shiftState = 0;  //integer state of knob as 0-8
int shiftChannelValues[6] = { 0, 0, 0, 0, 0, 0 }; //integer states to shift
int shiftKnobAnalogValueSteps[9] = { 0, 127, 254, 383, 511, 638, 767, 895, 1000 };



int Channel = 1; //active selected channel
int ClockState = 0; //whether or not clock input is high
int steps = 0; //beginning number of the steps in the sequence adjusted by StepLength


bool Sequence[12][43]; //max length is really 32 but shift knob enables some overflow
bool DefaultSequence[12][43];

void setup()
{

  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  pinMode(OUT5, OUTPUT);
  pinMode(OUT6, OUTPUT);
  pinMode(BankLED, OUTPUT);
  pinMode(ClockOut, OUTPUT);
  pinMode(ClockIn, INPUT);
  pinMode(BigButton, INPUT);
  pinMode(ButtonDelete, INPUT);
  pinMode(ButtonClear, INPUT);
  pinMode(ButtonBank, INPUT);
  pinMode(ResetButton, INPUT);
  pinMode(FillButton, INPUT);


  for (i = 0; i < 14; i++) {
    for (ii = 0; ii < 43; ii++) {
      Sequence[i][ii] = 0;
      DefaultSequence[i][ii] = 0;
    }
  }

  //show startup sequence
  blink(BankLED);
  blink(OUT1);
  blink(OUT2);
  blink(OUT3);
  blink(OUT4);
  blink(OUT5);
  blink(OUT6);

  //initialize analog values
  readStepsKnob();
  readChannelSelectKnob();
  readShiftKnob();

  attachInterrupt(ClockIn, isr, RISING);
}

int StepLength = 0; //analog value storage
void readStepsKnob(){
  StepLength = 1024 - analogRead(1);
  if (0 < StepLength) {         steps = 2; }
  else if (200 < StepLength) {  steps = 4; }
  else if (500 < StepLength) {  steps = 8; }
  else if (800 < StepLength) {  steps = 16;}
  else if (1000 < StepLength) { steps = 32;}
}

int CHANNELSELECT = 0; //analog value storage
void readChannelSelectKnob(){
  CHANNELSELECT= analogRead(0);
  if(20>CHANNELSELECT) {       Channel = 1; } 
  else if(170<CHANNELSELECT) { Channel = 2; } 
  else if(240<CHANNELSELECT) { Channel = 3; } 
  else if(420<CHANNELSELECT) { Channel = 4; } 
  else if(750<CHANNELSELECT) { Channel = 5; } 
  else if(1000<CHANNELSELECT){ Channel = 6; }
}


void readShiftKnob(){
  //SHIFT knob - note swap with commented line for reversed pot pins
  shiftKnobVal = 1024 - analogRead(2);
  //KnobVal = analogRead(2);
  
  int i = 0;
  while (i <= 8) {
    if (shiftKnobAnalogValueSteps[i] > shiftKnobVal) {
      shiftState = i;
      break;
    }
    i++;
  }
  
  int diff = abs(shiftKnobVal - shiftOldKnobVal);
  if (diff > TOLERANCE) {
    shiftChannelValues[Channel] = shiftState;
  }

  shiftOldKnobVal = shiftKnobVal;

}

void blink(int output) {
  digitalWrite(output, HIGH);
  delay(50);
  digitalWrite(output, LOW);
}


//  isr() - quickly handle interrupts from the clock input
void isr()
{
  ClockState = HIGH;
  digitalWrite(ClockOut, HIGH);
}

// cast true/false values to HIGH/LOW to simplify code
void digitalWriteCast(int pinNumber, bool pinState) {
  digitalWrite(pinNumber, pinState ? HIGH : LOW);
}

void readButtons(){
  // BigButtonState = digitalRead(BigButton);
  DeleteButtonState = digitalRead(ButtonDelete);
  ClearButtonState  = digitalRead(ButtonClear);
  BankButtonState   = digitalRead(ButtonBank);
  ResetButtonState  = digitalRead(ResetButton);
  FillButtonState   = digitalRead(FillButton);
  
  //Read big button input and write to current sequence step
  if (digitalRead(BigButton) != BigButtonState) {
    BigButtonState = BigButtonState == HIGH ? LOW : HIGH; //flip button state
    if (BigButtonState == HIGH) {
      Sequence[Channel + bankChannelStates[Channel]][(ClockKeep + shiftChannelValues[Channel]) % 32] = 1;
    }
    prevBigButtonState = BigButtonState;
  }

  delay (5);// necessary? todo delete

  //debounce bank button and latch state to channel
  if (BankButtonState != BankButtonPrevState && millis() - time > debounce) {
    BankButtonPrevState = BankButtonState;
    time = millis();
    if (BankButtonState == HIGH) {
      bankChannelStates[Channel] = bankChannelStates[Channel] == 1 ? 0 : 1; //flip bit and latch rather than assign from button state
    }
  }

  if (DeleteButtonState == HIGH) {
    Sequence[Channel + bankChannelStates[Channel]][looper + 1] = 0;
  } //This is the delete button

  //todo implement reset to default state with shift button
  if (ResetButtonState != prevResetButtonState) {
    if (ResetButtonState == HIGH) {
      looper = 0;
      ClockKeep = 0;
      for (i = 0; i < 6; i++) {
        bankChannelStates[i] = 0;
        shiftChannelValues[i] = 0;
      }
    }
    prevResetButtonState = ResetButtonState;
  }
}



void loop()
{
  if (ClockState == HIGH) {
      ClockKeep += 1;
      looper += 1;
      // for (i = 0; i < 6; i++) { //track each channel independently?
      //   shiftChannelValues[i] += 1;
      // }
      digitalWrite(ClockOut, HIGH); //clock rising edge
      for (i = 0; i < 6; i++) {
        //set each channel output gate high or low per current step (or high if fill button is pressed)
        digitalWriteCast(
          outputs[i], 
          Sequence[i + bankChannelStates[i]][(looper + shiftChannelValues[i]) % 32] || 
           (FillButtonState == HIGH && i == Channel));
      }

      if ((ClockKeep - 1) % 4 == 0) { //blink led every 4th step, typically quarter notes, invert for bank 2
        digitalWrite(BankLED, bankChannelStates[Channel] == 1 ? LOW : HIGH);
      } else {
        digitalWrite(BankLED, bankChannelStates[Channel] == 1 ? HIGH : LOW);
      }

      delay(CLOCK_PULSE_LENGTH);

      digitalWrite(ClockOut, LOW); //clock falling edge
      for (i = 0; i < 6; i++) {
        digitalWrite(outputs[i], LOW); //gate falling edge
      }
      ClockState = LOW; //ready for next clock pulse
  }

  
  readShiftKnob();

  readStepsKnob();

  readChannelSelectKnob();

  readButtons();
  
  if (looper >= steps) {
    looper = 0; //this bit starts the sequence over again
  }
  if (ClockKeep >= 32) {
    looper = 0;
    ClockKeep = 0;
  }
}


