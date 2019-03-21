#ifndef UI_H
#define UI_H

#include "differential-repeater.h"

#define TRACK_SELECT_1     0b1
#define TRACK_SELECT_2     0b10
#define TRACK_SELECT_3     0b100
#define TRACK_SELECT_4     0b1000
#define TRACK_SELECT_5     0b10000
#define TRACK_SELECT_6     0b11
#define TRACK_SELECT_7     0b101
#define TRACK_SELECT_8     0b1001
#define TRACK_SELECT_9     0b10001
#define TRACK_SELECT_10    0b110
#define TRACK_SELECT_11    0b1010
#define TRACK_SELECT_12    0b10010
#define TRACK_SELECT_13    0b1100
#define TRACK_SELECT_14    0b10100
#define TRACK_SELECT_15    0b11000
#define TRACK_SELECT_16    0b111
#define DOWN_CODE        0b100000
#define UP_CODE          0b1000000
#define PLAY_CODE        0b10000000
#define RECORD_CODE      0b100000000
#define STOP_CODE        0b110000000
#define SCALE_CODE       0b1000000000
#define UTIL_CODE        0b10000000000
#define ALGORITHM_CODE   0b100000000000
#define OPMODE_CODE      0b110000000000
#define LONG_TIME          3000
#define READ_REG           0xFFF
#define REC                8
#define SCALE              9
//#ifndef _BV
//#define _BV(bit) (1 << (bit)) 
//#endif

class ui {
  public:
    ui();
    void _begin(Engine * link);
    void readTouch(uint16_t touch);
    uint8_t getTrack(uint8_t track);
    uint8_t track;
    uint16_t regTouch;
    uint16_t reg12;
    uint8_t lowCode;
    uint8_t highCode;
    bool opMode = false;
    uint16_t getStuff();
    struct control {
      bool current:4;
      bool prev:3;
      bool held:1;
    };
    control _play      {.current = false, .prev = false, .held = false};
    control _record    {.current = false, .prev = false, .held = false};
    control _pause     {.current = false, .prev = false, .held = false};
    control _stop      {.current = false, .prev = false, .held = false};
    control _scale     {.current = false, .prev = false, .held = false};
    control _key       {.current = false, .prev = false, .held = false};
    control _up        {.current = false, .prev = false, .held = false};
    control _down      {.current = false, .prev = false, .held = false};
    control _construct {.current = false, .prev = false, .held = false};
    control _destruct  {.current = false, .prev = false, .held = false};
   
    
  private:
    unsigned long longTouch;
    //uint16_t reg12;
    bool tracks[16] = {false};
    Engine * sequencer;
    const uint8_t trackCode[18] =
        {0x1, 0x2, 0x4, 0x8, 0x10, 0x3, 0x5, 0x9, 0x11, 0x6, 0xA, 0x12, 0xC, 0x14, 0x18, 0x7, 0xE, 0x1C};

    
};

#endif
