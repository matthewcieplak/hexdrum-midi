//////
/////         H    H  EEEEE  X      X    DDDD    RRRR    U   U  M    M
/////        H    H  E        X   X     D  DD   R   R   U   U  M MM M
/////       HHHHHH  EEE        X       D    D  R RR    U   U  M  M M
/////      H    H  E         X  X     D   DD  R   R   U   U  M    M
/////     H    H  EEEEEE  X      X   DD DD   R     R   UU   M    M
/////
/////     HEX DRUM is a midi-capable expanded port of the "big button" by
/////        look mum no computer
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


const byte MIDI_CHANNEL = 16; //set this to receive midi note data on a specific channel 
//NOTE: non-zero indexed, range is 1-16 (not 0-15)

#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial midiSerial(0, 1); // RX, TX
// MIDI commands
#define MIDI_NOTE_ON 144
#define MIDI_NOTE_OFF 128

#define MIDI_CLOCK          0xF8
#define MIDI_CLOCK_START    0XFA
#define MIDI_CLOCK_CONTINUE 0xFB
#define MIDI_CLOCK_STOP     0xFC
#define MIDI_CLOCK_DIVISION 6    //MIDI uses 24 steps per quarter note, 6 steps is a sixteenth note. use smaller values for faster clock 
//MIDI states
#define STATE_NONE      0
#define STATE_NOTE_ON   1
#define STATE_NOTE_PLAY 2
#define STATE_NOTE_OFF  3
int midiState = STATE_NONE;
int midiClocks = 0;


const int midi_notes[6] = { //route input notes to output gates, standard GM midi drum mapping
  36, //C1 
  38, //D1
  40, //E1
  41, //F1
  43, //G1
  45  //A2
 };


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
long pulsekeeper;

//loop operators
int i;
int ii;

//Clock Reset Keepers
int ClockKeep = 0;
int looper = 0;
int step_duration_ms = 125; // 1/16th note at 120bpm is default - use for quantization
int step_duration_ms_half = 62;
long step_time = 0;
long button_time = 0;
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



int Channel = 0; //active selected channel
int ClockState = 0; //whether or not clock input is high
int PulseState = 0; //whether or not clock/gate output is high
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

  attachInterrupt(ClockIn, onClockIn, RISING);

  midiSerial.begin(31250);
}

int StepLength = 0; //analog value storage
void readStepsKnob(){
  StepLength = 1024 - analogRead(1);
  if      (StepLength < 200)  { steps = 2; }
  else if (StepLength < 500)  { steps = 4; }
  else if (StepLength < 800)  { steps = 8; }
  else if (StepLength < 1000) { steps = 16;}
  else                        { steps = 32;}
}

int CHANNELSELECT = 0; //analog value storage
void readChannelSelectKnob(){
  CHANNELSELECT= analogRead(0);
  if(CHANNELSELECT < 170) {       Channel = 0; }
  else if(CHANNELSELECT < 240) { Channel = 1; }
  else if(CHANNELSELECT < 420) { Channel = 2; }
  else if(CHANNELSELECT < 750) { Channel = 3; }
  else if(CHANNELSELECT < 1000){ Channel = 4; }
  else {                         Channel = 5; }
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


void onClockIn()
{
  ClockState = HIGH;
  digitalWrite(ClockOut, HIGH);

  //count tempo for quantization
  step_duration_ms = millis() - step_time;
  step_time = millis();
  step_duration_ms_half = step_duration_ms / 2;
}

// cast true/false values to HIGH/LOW to simplify code
void digitalWriteCast(int pinNumber, bool pinState) {
  digitalWrite(pinNumber, pinState ? HIGH : LOW);
}

void readButtons(){
  BigButtonState = digitalRead(BigButton);
  DeleteButtonState = digitalRead(ButtonDelete);
  ClearButtonState  = digitalRead(ButtonClear);
  BankButtonState   = digitalRead(ButtonBank);
  ResetButtonState  = digitalRead(ResetButton);
  FillButtonState   = digitalRead(FillButton);

  //Read big button input and write to nearest sequence step
  if (BigButtonState != prevBigButtonState) {
    prevBigButtonState = BigButtonState;
    if (BigButtonState == HIGH) {

      button_time = millis() - step_time; // get current step offset in ms
      int step_quantize_advance = 0;
      if (button_time > step_duration_ms_half) { //if we're closer to the current step activate and trigger
        step_quantize_advance = 1; //otherwise activate the next step
      } else {
        digitalWrite(outputs[Channel], HIGH); 
      }

      Sequence[Channel*2 + bankChannelStates[Channel]][(looper + shiftChannelValues[Channel] + step_quantize_advance) % steps] = 1;
    }
  }

  //delay (5);// necessary? todo delete

  if (ClearButtonState == HIGH) {
    for (i = 0; i < 32; i++) {
      Sequence[Channel*2 + bankChannelStates[Channel]][i] = 0;
    }
  }                                                 //This is the clear button


  //debounce bank button and latch state to channel
  if (BankButtonState != BankButtonPrevState && millis() - time > debounce) {
    BankButtonPrevState = BankButtonState;
    time = millis();
    if (BankButtonState == HIGH) {
      bankChannelStates[Channel] = bankChannelStates[Channel] == 1 ? 0 : 1; //flip bit and latch rather than assign from button state
    }
  }

  if (DeleteButtonState == HIGH) {
    Sequence[Channel*2 + bankChannelStates[Channel]][looper + 1] = 0;
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



 // received MIDI data
byte midiByte;
byte midiChannel;
byte midiCommand;
byte midiNote;
byte midiVelocity;

void receiveMIDI(){
 if (midiSerial.available() > 0) {
        // read MIDI byte
        midiByte = midiSerial.read();
        switch (midiState) {
        case STATE_NONE:
            // remove channel info
            midiChannel = midiByte & B00001111;
            midiCommand = midiByte & B11110000;
            if (midiChannel == MIDI_CHANNEL - 1){
                if (midiCommand == MIDI_NOTE_ON){
                    midiState = STATE_NOTE_ON;
                } else  if (midiCommand == MIDI_NOTE_OFF) {
                    midiState = STATE_NOTE_OFF;
                }
            } else if (midiByte == MIDI_CLOCK_START || midiByte == MIDI_CLOCK_CONTINUE) {
                // midiState = STATE_NONE;
                midiClocks = 0;
                looper = 0;
                ClockKeep = 0;
                onClockIn();
            } else if (midiByte == MIDI_CLOCK) {
                midiClocks += 1;
                // digitalWrite(BankLED, (midiClocks % 2) > 0 ? HIGH : LOW);
                if (midiClocks == MIDI_CLOCK_DIVISION) {
                  onClockIn();
                  midiClocks = 0;
                }
              // midiState = STATE_CLOCK_START;
            } else if (midiCommand == MIDI_CLOCK_STOP) {
                midiClocks = 0;
                // looper = 0;
                // ClockKeep = 0;
            } 
            break;


        case STATE_NOTE_ON:
            midiNote = midiByte;
            midiState = STATE_NOTE_PLAY;
            break;
            
        case STATE_NOTE_OFF:
            midiNote = midiByte;
        case STATE_NOTE_PLAY:
            midiVelocity = midiByte;
            
            if (midiVelocity > 0 && midiState == STATE_NOTE_PLAY)
            {
                // digitalWrite(BankLED, HIGH);
                for (i = 0; i < 6; i++) {
                  if (midiNote == midi_notes[i]) {
                    digitalWrite(outputs[i], HIGH);
                    break;
                  }
                }
            } else { //receiving a note off message or zero velocity message
                // digitalWrite(BankLED, LOW);
                for (i = 0; i < 6; i++) {
                  if (midiNote == midi_notes[i]) {
                    digitalWrite(outputs[i], LOW);
                    break;
                  }
                }
            }

            midiState = STATE_NONE;            
            break;
            
        } // switch

    } // mySerial.available()

}

void loop()
{
  if (ClockState == HIGH && PulseState == LOW) {
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
          Sequence[i*2 + bankChannelStates[i]][(looper + shiftChannelValues[i]) % steps] ||
           (FillButtonState == HIGH && i == Channel));
      }

      if ((ClockKeep - 1) % 4 == 0) { //blink led every 4th step, typically quarter notes, invert for bank 2
        digitalWrite(BankLED, bankChannelStates[Channel] == 1 ? LOW : HIGH);
      } else {
        digitalWrite(BankLED, bankChannelStates[Channel] == 1 ? HIGH : LOW);
      }

      ClockState = LOW;
      PulseState = HIGH;
      pulsekeeper = millis();
      //delay(CLOCK_PULSE_LENGTH); //remove delay calls to fix serial input
  }

  if (PulseState == HIGH && millis() - pulsekeeper > CLOCK_PULSE_LENGTH){
      digitalWrite(ClockOut, LOW); //clock falling edge
      for (i = 0; i < 6; i++) {
        digitalWrite(outputs[i], LOW); //gate falling edge
      }
      PulseState = LOW; //ready for next clock pulse
  }


  readShiftKnob();

  readStepsKnob();

  readChannelSelectKnob();

  readButtons();

  receiveMIDI();

  if (looper >= steps) {
    looper = 0; //this bit starts the sequence over again
  }
  if (ClockKeep >= 32) {
    looper = 0;
    ClockKeep = 0;
  }
}
