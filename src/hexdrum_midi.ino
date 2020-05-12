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
const int BankLED = A6;//17
const int ClockOut = A3;

#define TOLERANCE 0

int buttonPushCounter = 0;   // counter for the number of button presses
volatile int buttonState = LOW;         // current state of the button
int lastButtonState = 0;     // previous state of the button
int RecordButtonState = 0;
int LastRecordButtonState = 0;
int DeleteButtonState = 0;
int LastDeleteButtonState = 0;
int ClearButtonState = 0;

//Clock Reset Keepers
int ClockKeep = 0;
int ResetSteps = 33;

//RESET BUTTON
int ResetButton = 6;
int ResetButtonState = 0;
int LastResetButtonState = 0;

int CHANNELSELECT = 0;


//BeatShiftingPot
int PotValue1 = 0;
int PotValue2 = 0;
int PotValue3 = 0;
int PotValue4 = 0;
int PotValue5 = 0;
int PotValue6 = 0;
int Potval;
int UnscaledPotval = 0;

//FILL BUTTON
int FillButton = 5;
int FillButtonState = 0;
int Fill1 = 0;
int Fill2 = 0;
int Fill3 = 0;
int Fill4 = 0;
int Fill5 = 0;
int Fill6 = 0;

//CLEAR

int ClearState = 0;

//Bank Button Latching
long time = 0;
long debounce = 150;
int ButtonBankSelectState[7];
int BankState[7] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW};
int Bankprevious[7] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

int BankArrayShift1 = 0;
int BankArrayShift2 = 0;
int BankArrayShift3 = 0;
int BankArrayShift4 = 0;
int BankArrayShift5 = 0;
int BankArrayShift6 = 0;
int BankClear = 0;
int BankRecord = 0;
int BankArrayNumber = 0;
int ButtonBankSelectButton = 0;

//SHIFT KNOB

int KnobValue = 0;
int BankPush1 = 0;
int BankPush2 = 0;
int BankPush3 = 0;
int BankPush4 = 0;
int BankPush5 = 0;
int BankPush6 = 0;
int KnobVal = 0;
int OldKnobValue = 0;
int NewKnobValue = 0;
int NewKnobValue1 = 0;
int NewKnobValue2 = 0;
int NewKnobValue3 = 0;
int NewKnobValue4 = 0;
int NewKnobValue5 = 0;
int NewKnobValue6 = 0;



int looper = 0;
int Channel = 1;
int ClockState = 0;            //clock state stuff
int StepLength = 0;           //What the pot uses for step length
int steps = 0;              //beginning number of the steps in the sequence adjusted by StepLength
const int clkIn = 2;
int BigButton = A5;

int ButtonClear = 4;         //reset button for the moment
int ButtonDelete = 7;
int ButtonBankSelect = 3;

int ChannelSelectState1 = 0;
int ChannelSelectState2 = 0;
int ChannelSelectState3 = 0;
int ChannelSelectState4 = 0;
int ChannelSelectState5 = 0;
int ChannelSelectState6 = 0;

int shiftKnobAnalogValues[8] = { 0, 127, 254, 383, 511, 638, 767, 895 };




int Sequence[14][43];
int DefaultSequence[14][43];

  //sequence array 1

};//sequence array 1




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
  pinMode(clkIn, INPUT);
  pinMode(BigButton, INPUT);
  pinMode(ButtonDelete, INPUT);
  pinMode(ButtonClear, INPUT);

  pinMode(ButtonBankSelect, INPUT);
  pinMode(ResetButton, INPUT);
  pinMode(FillButton, INPUT);

  int i;
  int ii;
  for (i = 0; i < 14; i++) {
    for (ii = 0; ii < 43; ii++) {
      Sequence[i][ii] = 0;
      DefaultSequence[i][ii] = 0;
    }
  }

  
  StepLength = 1024 - analogRead(1);
  if (0 < StepLength) {
    steps = 2;
  }
  if (200 < StepLength) {
    steps = 4;
  }
  if (500 < StepLength) {
    steps = 8;
  }
  if (800 < StepLength) {
    steps = 16;
  }
  if (1000 < StepLength) {
    steps = 32;
  }
  //making sure it starts up knowing where the knob is


  CHANNELSELECT = analogRead(2);



  blink(BankLED);
  blink(OUT1);
  blink(OUT2);
  blink(OUT3);
  blink(OUT4);
  blink(OUT5);
  blink(OUT6);


  attachInterrupt(clkIn, isr, RISING);


}

void blink(int output) {
  digitalWrite(output, HIGH);
  delay(50);
  digitalWrite(output, LOW);
}



void loop()
{



  RecordButtonState = digitalRead(BigButton);
  DeleteButtonState = digitalRead(ButtonDelete);
  ClearButtonState = digitalRead(ButtonClear);
  ResetButtonState = digitalRead(ResetButton);
  FillButtonState = digitalRead(FillButton);
  ButtonBankSelectState[BankArrayNumber] = digitalRead(ButtonBankSelect);//These setup the states of the buttons


{ if (buttonState == HIGH) {

      looper    = (looper + 1);
      BankPush1 = (BankPush1 + 1);
      BankPush2 = (BankPush2 + 1);
      BankPush3 = (BankPush3 + 1);
      BankPush4 = (BankPush4 + 1);
      BankPush5 = (BankPush5 + 1);
      BankPush6 = (BankPush6 + 1);
      ClockKeep = (ClockKeep + 1);
      digitalWrite(ClockOut, HIGH);
      digitalWriteCast(OUT1, Sequence[1 + BankArrayShift1][BankPush1 + NewKnobValue1] || (Fill1));
      digitalWriteCast(OUT2, Sequence[2 + BankArrayShift2][BankPush2 + NewKnobValue2] || (Fill2));
      digitalWriteCast(OUT3, Sequence[3 + BankArrayShift3][BankPush3 + NewKnobValue3] || (Fill3));
      digitalWriteCast(OUT4, Sequence[7 + BankArrayShift4][BankPush4 + NewKnobValue4] || (Fill4));
      digitalWriteCast(OUT5, Sequence[8 + BankArrayShift5][BankPush5 + NewKnobValue5] || (Fill5));
      digitalWriteCast(OUT6, Sequence[9 + BankArrayShift6][BankPush6 + NewKnobValue6] || (Fill6));
      delay(10);
      digitalWrite(ClockOut, LOW);
      digitalWrite(OUT1, LOW);
      digitalWrite(OUT2, LOW);
      digitalWrite(OUT3, LOW);
      digitalWrite(OUT4, LOW);
      digitalWrite(OUT5, LOW);
      digitalWrite(OUT6, LOW);
      buttonState = LOW;
    }
    else
    {
      looper = looper;
      ClockKeep = ClockKeep;
    }
  }



  if (RecordButtonState != LastRecordButtonState) {
    if ((RecordButtonState == HIGH) && (Channel == 1)) {
      Sequence[Channel + BankRecord][BankPush1 + 1 + NewKnobValue1] = 1;
    }
    else if ((RecordButtonState == HIGH) && (Channel == 2)) {
      Sequence[Channel + BankRecord][BankPush2 + 1 + NewKnobValue2] = 1;
    }
    else if ((RecordButtonState == HIGH) && (Channel == 3)) {
      Sequence[Channel + BankRecord][BankPush3 + 1 + NewKnobValue3] = 1;
    }
    else if ((RecordButtonState == HIGH) && (Channel == 7)) {
      Sequence[Channel + BankRecord][BankPush4 + 1 + NewKnobValue4] = 1;
    }
    else if ((RecordButtonState == HIGH) && (Channel == 8)) {
      Sequence[Channel + BankRecord][BankPush5 + 1 + NewKnobValue5] = 1;
    }
    else if ((RecordButtonState == HIGH) && (Channel == 9)) {
      Sequence[Channel + BankRecord][BankPush6 + 1 + NewKnobValue6] = 1;
    }


    else {
    }
    delay (5);
  }                                                                   //RECORD BUTTON



  //This bit is the clock in and step advance stuff



  
  
  if ((ClockKeep == 1)  || (ClockKeep == 5) || (ClockKeep == 9)  || (ClockKeep == 13) ||  (ClockKeep == 17)  || (ClockKeep == 21) || (ClockKeep == 25) || (ClockKeep == 29)) {
      if (BankRecord == 3) {
        digitalWrite(BankLED, LOW);
      } else {
        digitalWrite(BankLED, HIGH);
      }
  } else {
    digitalWriteCast(BankLED, BankState[BankArrayNumber]);
  }



  if (ButtonBankSelectState[BankArrayNumber] == HIGH && Bankprevious[BankArrayNumber] == LOW && millis() - time > debounce) {
    if (BankState[BankArrayNumber] == HIGH) {
      BankState[BankArrayNumber] = LOW;
    }
    else {
      BankState[BankArrayNumber] = HIGH;
      time = millis();
    }
  }
  //Bank button state making it latch and stuff


  //SHIFT knob - note swap with commented line for reversed pot pins
  KnobVal = 1024 - analogRead(2);
  //KnobVal = analogRead(2);
  
  int i = 0;
  while (i < 8) {
    if (shiftKnobAnalogValues[i] > KnobVal) {
      KnobValue = i-1;
      break;
    }
    i++;
  }
  if (1000 < KnobVal) {
    KnobValue = 8;
  }

  int diff = abs(KnobValue - OldKnobValue);
  if (diff > TOLERANCE) {
    if (ChannelSelectState1 == HIGH) NewKnobValue1 = KnobValue;
    if (ChannelSelectState2 == HIGH) NewKnobValue2 = KnobValue;
    if (ChannelSelectState3 == HIGH) NewKnobValue3 = KnobValue;
    if (ChannelSelectState4 == HIGH) NewKnobValue4 = KnobValue;
    if (ChannelSelectState5 == HIGH) NewKnobValue5 = KnobValue;
    if (ChannelSelectState6 == HIGH) NewKnobValue6 = KnobValue;
    
    OldKnobValue = KnobValue;
  }




//  CHANNELSELECT = analogRead(0);
//    (ChannelSelectState1 = LOW);
//    (ChannelSelectState2 = LOW);
//    (ChannelSelectState3 = LOW);
//    (ChannelSelectState4 = LOW);
//    (ChannelSelectState5 = LOW);
//    (ChannelSelectState6 = LOW);
//    if (20 > CHANNELSELECT)   {
//      (ChannelSelectState1 = HIGH);
//    } 
//    if (170 < CHANNELSELECT) {
//      (ChannelSelectState2 = HIGH);
//    } 
//    if (240 < CHANNELSELECT) {
//      (ChannelSelectState3 = HIGH);
//    } 
//    if (420 < CHANNELSELECT) {
//      (ChannelSelectState4 = HIGH);
//    } 
//    if (750 < CHANNELSELECT) {
//      (ChannelSelectState5 = HIGH);
//    } 
//    if (1000 < CHANNELSELECT) {
//      (ChannelSelectState6 = HIGH);
//    }

{CHANNELSELECT= analogRead(0);
if(20>CHANNELSELECT)   {(ChannelSelectState1 = HIGH); (ChannelSelectState2 = LOW); (ChannelSelectState3 = LOW); (ChannelSelectState4 = LOW); (ChannelSelectState5 = LOW);  (ChannelSelectState6 = LOW);}
if(170<CHANNELSELECT) {(ChannelSelectState1 = LOW); (ChannelSelectState2 = HIGH); (ChannelSelectState3 = LOW); (ChannelSelectState4 = LOW); (ChannelSelectState5 = LOW);  (ChannelSelectState6 = LOW);}
if(240<CHANNELSELECT) {(ChannelSelectState1 = LOW); (ChannelSelectState2 = LOW); (ChannelSelectState3 = HIGH); (ChannelSelectState4 = LOW); (ChannelSelectState5 = LOW);  (ChannelSelectState6 = LOW);}
if(420<CHANNELSELECT) {(ChannelSelectState1 = LOW); (ChannelSelectState2 = LOW); (ChannelSelectState3 = LOW);  (ChannelSelectState4 = HIGH);(ChannelSelectState5 = LOW);  (ChannelSelectState6 = LOW);}
if(750<CHANNELSELECT) {(ChannelSelectState1 = LOW); (ChannelSelectState2 = LOW); (ChannelSelectState3 = LOW);  (ChannelSelectState4 = LOW); (ChannelSelectState5 = HIGH); (ChannelSelectState6 = LOW);}
if(1000<CHANNELSELECT){(ChannelSelectState1 = LOW); (ChannelSelectState2 = LOW); (ChannelSelectState3 = LOW);  (ChannelSelectState4 = LOW); (ChannelSelectState5 = LOW);  (ChannelSelectState6 = HIGH);}
}






  if ((BankState [1] == LOW) && (ChannelSelectState1  == HIGH))                                  {
    Channel = 1;
    BankArrayNumber = 1;
    BankArrayShift1 = 0;
    BankRecord = 0;
    ClearState = 1;
  }
  else if ((BankState [2] == LOW) && (ChannelSelectState2  == HIGH))                                  {
    Channel = 2;
    BankArrayNumber = 2;
    BankArrayShift2 = 0;
    BankRecord = 0;
    ClearState = 2;
  }
  else if ((BankState [3] == LOW) && (ChannelSelectState3  == HIGH))                                  {
    Channel = 3;
    BankArrayNumber = 3;
    BankArrayShift3 = 0;
    BankRecord = 0;
    ClearState = 3;
  }
  else if ((BankState [4] == LOW) && (ChannelSelectState4  == HIGH))                                  {
    Channel = 7;
    BankArrayNumber = 4;
    BankArrayShift4 = 0;
    BankRecord = 0;
    ClearState = 7;
  }
  else if ((BankState [5] == LOW) && (ChannelSelectState5  == HIGH))                                  {
    Channel = 8;
    BankArrayNumber = 5;
    BankArrayShift5 = 0;
    BankRecord = 0;
    ClearState = 8;
  }
  else if ((BankState [6] == LOW) && (ChannelSelectState6  == HIGH))                                  {
    Channel = 9;
    BankArrayNumber = 6;
    BankArrayShift6 = 0;
    BankRecord = 0;
    ClearState = 9;
  }


  else if ((BankState [1] == HIGH) && (ChannelSelectState1  == HIGH))                                 {
    Channel = 1;
    BankArrayNumber = 1;
    BankArrayShift1 = 3;
    BankRecord = 3;
    ClearState = 1;
  }
  else if ((BankState [2] == HIGH) && (ChannelSelectState2  == HIGH))                                 {
    Channel = 2;
    BankArrayNumber = 2;
    BankArrayShift2 = 3;
    BankRecord = 3;
    ClearState = 2;
  }
  else if ((BankState [3] == HIGH) && (ChannelSelectState3  == HIGH))                                 {
    Channel = 3;
    BankArrayNumber = 3;
    BankArrayShift3 = 3;
    BankRecord = 3;
    ClearState = 3;
  }
  else if ((BankState [4] == HIGH) && (ChannelSelectState4  == HIGH))                                 {
    Channel = 7;
    BankArrayNumber = 4;
    BankArrayShift4 = 3;
    BankRecord = 3;
    ClearState = 7;
  }
  else if ((BankState [5] == HIGH) && (ChannelSelectState5  == HIGH))                                 {
    Channel = 8;
    BankArrayNumber = 5;
    BankArrayShift5 = 3;
    BankRecord = 3;
    ClearState = 8;
  }
  else if ((BankState [6] == HIGH) && (ChannelSelectState6  == HIGH))                                 {
    Channel = 9;
    BankArrayNumber = 6;
    BankArrayShift6 = 3;
    BankRecord = 3;
    ClearState = 9;
  }

  //switch for selecting between both channels and banks




  if (ClearButtonState == HIGH) {
    for (int i = 1; i < 42; i++) {
      Sequence[ClearState + BankRecord][i] = 0;
    }
  }                                                 //This is the clear button





  //for(int ii = 1; ii<14; ii++){
  //Sequence[ii][33]=Sequence[ii][1];
  //Sequence[ii][34]=Sequence[ii][2];
  //Sequence[ii][35]=Sequence[ii][3];
  //Sequence[ii][36]=Sequence[ii][4];
  //Sequence[ii][37]=Sequence[ii][5];
  //Sequence[ii][38]=Sequence[ii][6];
  //Sequence[ii][39]=Sequence[ii][7];
  //Sequence[ii][40]=Sequence[ii][8];
  //Sequence[ii][41]=Sequence[ii][9];
  //}                                    //THIS MAKES SURE WHEN THE KNOB SHIFTS THE PATTERN IT DOESNT LOSE A LOAD OF IT!



  if ((FillButtonState == HIGH) && (Channel == 1)) {
    Fill1 = 1;
    Fill2 = 0;
    Fill3 = 0;
    Fill4 = 0;
    Fill5 = 0;
    Fill6 = 0;
  }
  else if ((FillButtonState == HIGH) && (Channel == 2)) {
    Fill1 = 0;
    Fill2 = 1;
    Fill3 = 0;
    Fill4 = 0;
    Fill5 = 0;
    Fill6 = 0;
  }
  else if ((FillButtonState == HIGH) && (Channel == 3)) {
    Fill1 = 0;
    Fill2 = 0;
    Fill3 = 1;
    Fill4 = 0;
    Fill5 = 0;
    Fill6 = 0;
  }
  else if ((FillButtonState == HIGH) && (Channel == 7)) {
    Fill1 = 0;
    Fill2 = 0;
    Fill3 = 0;
    Fill4 = 1;
    Fill5 = 0;
    Fill6 = 0;
  }
  else if ((FillButtonState == HIGH) && (Channel == 8)) {
    Fill1 = 0;
    Fill2 = 0;
    Fill3 = 0;
    Fill4 = 0;
    Fill5 = 1;
    Fill6 = 0;
  }
  else if ((FillButtonState == HIGH) && (Channel == 9)) {
    Fill1 = 0;
    Fill2 = 0;
    Fill3 = 0;
    Fill4 = 0;
    Fill5 = 0;
    Fill6 = 1;
  }

  else if ((FillButtonState == LOW)  && (Channel == 1)) {
    Fill1 = 0;
    Fill2 = 0;
    Fill3 = 0;
    Fill4 = 0;
    Fill5 = 0;
    Fill6 = 0;
  }
  else if ((FillButtonState == LOW)  && (Channel == 2)) {
    Fill1 = 0;
    Fill2 = 0;
    Fill3 = 0;
    Fill4 = 0;
    Fill5 = 0;
    Fill6 = 0;
  }
  else if ((FillButtonState == LOW)  && (Channel == 3)) {
    Fill1 = 0;
    Fill2 = 0;
    Fill3 = 0;
    Fill4 = 0;
    Fill5 = 0;
    Fill6 = 0;
  }
  else if ((FillButtonState == LOW)  && (Channel == 7)) {
    Fill1 = 0;
    Fill2 = 0;
    Fill3 = 0;
    Fill4 = 0;
    Fill5 = 0;
    Fill6 = 0;
  }
  else if ((FillButtonState == LOW)  && (Channel == 8)) {
    Fill1 = 0;
    Fill2 = 0;
    Fill3 = 0;
    Fill4 = 0;
    Fill5 = 0;
    Fill6 = 0;
  }
  else if ((FillButtonState == LOW)  && (Channel == 9)) {
    Fill1 = 0;
    Fill2 = 0;
    Fill3 = 0;
    Fill4 = 0;
    Fill5 = 0;
    Fill6 = 0;
  }





  //This is the FILL button




  if (DeleteButtonState == HIGH) {
    Sequence[Channel + BankRecord][looper + 1] = 0;
  }                             //This is the delete button

  if (ResetButtonState != LastResetButtonState) {
    if (ResetButtonState == HIGH) {
      looper = 0;
      ClockKeep = 0;
      BankPush1 = 0;
      BankPush2 = 0;
      BankPush3 = 0;
      BankPush4 = 0;
      BankPush5 = 0;
      BankPush6 = 0;
    }
    LastResetButtonState = ResetButtonState;
  }



  //NOTE for potentiometer pins reversed, swap for commented line below
  StepLength = 1024 - analogRead(1);
  //StepLength = analogRead(1);

  if (0 < StepLength) {
    steps = 1;
  }
  if (150 < StepLength) {
    steps = 2;
  }
  if (300 < StepLength) {
    steps = 4;
  }
  if (500 < StepLength) {
    steps = 8;
  }
  if (750 < StepLength) {
    steps = 16;
  }
  if (1000 < StepLength) {
    steps = 32;
  }                                      //this bit chooses how long the sequence is





  if (looper >= steps) {
    looper = 0; //this bit starts the sequence over again
  }
  if (ClockKeep >= 32) {
    looper = 0;
    ClockKeep = 0;
  }
  if ((BankPush1  + NewKnobValue1) >= steps) {
    BankPush1 = 0;
  }
  if ((BankPush2  + NewKnobValue2) >= steps) {
    BankPush2 = 0;
  }
  if ((BankPush3  + NewKnobValue3) >= steps) {
    BankPush3 = 0;
  }
  if ((BankPush4  + NewKnobValue4) >= steps) {
    BankPush4 = 0;
  }
  if ((BankPush5  + NewKnobValue5) >= steps) {
    BankPush5 = 0;
  }
  if ((BankPush6  + NewKnobValue6) >= steps) {
    BankPush6 = 0;
  }






  lastButtonState = buttonState;
  LastRecordButtonState = RecordButtonState;
  LastResetButtonState = ResetButtonState;//sectoion is for the state change detections
  Bankprevious[BankArrayNumber] = ButtonBankSelectState[BankArrayNumber];
}



//  =================== convenience routines ===================

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{

  buttonState = HIGH;
  digitalWrite(ClockOut, HIGH);


}

void digitalWriteCast(int pinNumber, bool pinState) {
  digitalWrite(pinNumber, pinState ? HIGH : LOW);
}
