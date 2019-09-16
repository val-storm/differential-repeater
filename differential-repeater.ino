#include <Wire.h>
#include "Adafruit_MPR121.h"
#include "differential-repeater.h"
#include "ui.h"
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <MIDI.h>
#include <Encoder.h>

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

Adafruit_MPR121 keys = Adafruit_MPR121();
Adafruit_MPR121 controls = Adafruit_MPR121();
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// State change variable
uint16_t lasttouchedKeys = 0;
uint16_t currtouchedKeys = 0;
uint16_t controlRegister = 0;
uint16_t controlState = 0;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial5, MIDI); 

//long touch variables
unsigned long _now;
uint8_t counter = 0;
bool tripped = false;

Engine starfield;

ui touchUI;

int latchPin = 1;
int clockPin = 0;
int dataPin = 32;
 
uint8_t leds = 0;

Encoder knobRight(27, 26);
Encoder knobLeft(24, 6);
int8_t positionLeft  = -127;
int8_t positionRight = -127;


void setup() {
  
  //Serial.begin(9600);

  MIDI.begin();
  
//  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
//    delay(10);
//  }
  
//  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!keys.begin(0x5A)) {
//    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
//  Serial.println("MPR121 found!");
  if (!controls.begin(0x5C)) {
//    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
 // Serial.println("MPR121 found!");

  //init display
  alpha4.begin(0x70);

  alpha4.writeDisplay();
  
  //init sequencer engine
  starfield._begin();

  //pass the main sketch midi method to the sequencer 
  starfield.setMidiHandler(midiOut);

  starfield.setStepSignal(displayStep);

  //pass the sequencer class instance to the UI
  touchUI._begin(& starfield);
  
  _now = millis();

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);

//  starfield.setLoopPoint(2, 31);
//  starfield.setLoopPoint(0, 63);
//  starfield.setLoopPoint(3, 77);
//  starfield.setLoopPoint(9, 14);
//  starfield.setLoopPoint(1, 16);

/*
 * HARDCODED PERFORMANCE MODE
 */

  starfield.setDivision(1, 0);
  starfield.setDivision(0, 1);
  starfield.setDivision(2, 2);
  starfield.setDivision(3, 3);
  starfield.setDivision(1, 4);
  starfield.setDivision(0, 5);
  starfield.setDivision(2, 6);
  starfield.setDivision(3, 7);
  starfield.setDivision(0, 8);
  starfield.setDivision(3, 9);
  starfield.setDivision(1, 10);
  starfield.setDivision(1, 11);
  starfield.setDivision(0, 12);
  starfield.setDivision(0, 13);
  starfield.setDivision(2, 14);
  starfield.setDivision(3, 15);

  starfield.assignMidi(0, 1);
  starfield.assignMidi(1, 1);
  starfield.assignMidi(2, 1);
  starfield.assignMidi(3, 1);
  starfield.assignMidi(4, 2);
  starfield.assignMidi(5, 2);
  starfield.assignMidi(6, 2);
  starfield.assignMidi(7, 2);
  starfield.assignMidi(8, 3);
  starfield.assignMidi(9, 3);
  starfield.assignMidi(10, 10);
  starfield.assignMidi(11, 10);
  starfield.assignMidi(12, 10);
  starfield.assignMidi(13, 10);
  starfield.assignMidi(14, 10);
  starfield.assignMidi(15, 10);

  starfield.setTrackScale(1, 0);
  starfield.setTrackScale(1, 1);
  starfield.setTrackScale(1, 2);
  starfield.setTrackScale(1, 3);
  starfield.setTrackScale(1, 4);
  starfield.setTrackScale(1, 5);
  starfield.setTrackScale(1, 6);
  starfield.setTrackScale(1, 7);
  starfield.setTrackScale(1, 8);
  starfield.setTrackScale(1, 9);
  starfield.setTrackScale(12, 10);
  starfield.setTrackScale(12, 11);
  starfield.setTrackScale(12, 12);
  starfield.setTrackScale(12, 13);
  starfield.setTrackScale(12, 14);
  starfield.setTrackScale(12, 15);


}

void loop() {
  
  //check in with the sequencer engine
  starfield.transport(); 
  
  // Get the currently touched pads
  currtouchedKeys = keys.touched();
  controlRegister = controls.touched();

  //if the control keys have changed see what actions to take
  if(controlRegister != controlState)
  {
    touchUI.readTouch(controlRegister);
    controlState = controlRegister;
  }

  //check keyboard status for note on/off changes
  for (uint8_t i=0; i<12; i++) {
    if ((currtouchedKeys & _BV(i)) && !(lasttouchedKeys & _BV(i)) ) {
      starfield.writeNoteOn(i);
    }
    if (!(currtouchedKeys & _BV(i)) && (lasttouchedKeys & _BV(i)) ) {
      starfield.writeNoteOff(i);
    }
  }
  
  // update state of keyboard
  lasttouchedKeys = currtouchedKeys;

  leds = starfield.getRecording();

  updateShiftRegister();

//  for(uint8_t i = 0; i < 16; i++)
//  {
//    Serial.print(starfield.getWrite(i));
//  }
//  Serial.println();
  int8_t newLeft, newRight;
  newLeft = knobLeft.read();
  newRight = knobRight.read();
  if (newLeft != positionLeft || newRight != positionRight) {
    positionLeft = newLeft;
    positionRight = newRight;
    starfield.setTempo(DEFAULT_TEMPO + positionRight);
    starfield.setLoopPoint(DEFAULT_LOOP_POINT + positionLeft);
   // Serial.println(positionRight);
   // Serial.println(starfield.getPosition(0));
  }

    //Serial.println(controlRegister);
    //delay(100);
  return;
  /*
  // debugging info, what
  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0b"); Serial.println(controls.touched(), BIN);
  
  Serial.print("Filt: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(keys.filteredData(i)); Serial.print("\t");
  }
  Serial.println();
  Serial.print("Base: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(keys.baselineData(i)); Serial.print("\t");
  }
  
  Serial.println();
  
  Serial.print("UI reg: ");
  Serial.print(touchUI.getStuff(), BIN);

  Serial.println();
  
  Serial.print("int reg: ");
  Serial.print(controlRegister, BIN);
  Serial.println();
  // put a delay so it isn't overwhelming
  delay(100);
  */
}

void midiOut(uint8_t note, uint8_t type, uint8_t channel)
{
  
  if(type == 1)
  {
    MIDI.sendNoteOn(note, 99, channel);
    usbMIDI.sendNoteOn(note, 99, channel);
    //alpha4.writeDigitRaw(0, type);
  }
  
  if(type == 0)
  {
    MIDI.sendNoteOff(note, 0, channel);
    usbMIDI.sendNoteOff(note, 0, channel);
    //alpha4.writeDigitRaw(0, type);
  }
  
  
  
}

void displayStep()
{
  for(uint8_t i = 0; i < 4; i++)
  {
    alpha4.writeDigitRaw(i, random(65535));
  }
  alpha4.writeDisplay();
}

void updateShiftRegister()
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, leds);
   digitalWrite(latchPin, HIGH);
}
