/*********************************************************
This is a library for the MPR121 12-channel Capacitive touch sensor

Designed specifically to work with the MPR121 Breakout in the Adafruit shop 
  ----> https://www.adafruit.com/products/

These sensors use I2C communicate, at least 2 pins are required 
to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.  
BSD license, all text above must be included in any redistribution
**********************************************************/

#include <Wire.h>
#include "Adafruit_MPR121.h"
#include "differential-repeater.h"
#include "ui.h"

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 keys = Adafruit_MPR121();
Adafruit_MPR121 controls = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouchedKeys = 0;
uint16_t currtouchedKeys = 0;
//uint16_t lasttouchedInterface = 0;
uint16_t controlRegister = 0;
uint16_t keysRegister = 0;
uint16_t controlState = 0;
uint16_t keysState = 0;

unsigned long _now;
uint8_t counter = 0;
bool tripped = false;

Engine starfield = Engine();

ui touchUI;

void setup() {
  
  

  Serial.begin(9600);

  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!keys.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  if (!controls.begin(0x5C)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  starfield._begin();
  starfield.setMidiHandler(midiOut);
  touchUI._begin(& starfield);
  
  _now = millis();

   for(uint8_t x = 0; x < 13; x++)
   {
    for(uint8_t y = 0; y < 12; y++)
    {
      Serial.print(starfield.printScales(x,y));
    }
    Serial.println(" ");
   } 
}

void loop() {
 
  starfield.transport(); 
  
  // Get the currently touched pads
  //keysRegister = keys.touched();
  currtouchedKeys = keys.touched();
  controlRegister = controls.touched();

  if(controlRegister != controlState)
  {
    touchUI.readTouch(controlRegister);
    controlState = controlRegister;
  }


  
//  if(keysRegister != keysState)
//  {
//    starfield.writeNote(keysRegister);
//    keysState = keysRegister;
//  }

 

//  for (uint8_t i = 0; i < 12; i++)
//  {
//    Serial.print((keysRegister &  (1 << i)) >> i);
//  }
//  Serial.println();
//  for (uint8_t i = 0; i < 12; i++)
//  {
//    Serial.print(keysState & (1 << i));
//  }
//
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouchedKeys & _BV(i)) && !(lasttouchedKeys & _BV(i)) ) {
     // Serial.print(i); Serial.println(" touched");
      starfield.writeNoteOn(i);
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouchedKeys & _BV(i)) && (lasttouchedKeys & _BV(i)) ) {
     // Serial.print(i); Serial.println(" released");
      starfield.writeNoteOff(i);
    }
  }
  
  // reset our state
  lasttouchedKeys = currtouchedKeys;

  for(uint8_t i = 0; i < 16; i++)
  {
    Serial.print(starfield.getPosition(i));
  }
  
  Serial.println();
  for(uint8_t i = 0; i < 16; i++)
  {
    Serial.print(starfield.getWrite(i));
  }
  
  Serial.println();
  Serial.println(touchUI.getStuff(), BIN);
  //delay(10);
  // comment out this line for detailed data from the sensor!
  //return;
  
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
}

void midiOut(uint8_t note, uint8_t type, uint8_t channel)
{
  
  if(type == 1)
  {
    usbMIDI.sendNoteOn(note, 99, channel);
   //Serial.println(note);
  }
  
  if(type == 0)
  {
    usbMIDI.sendNoteOff(note, 0, channel);
   // Serial.println("Note Off sent");
  }
  
  
  
}

