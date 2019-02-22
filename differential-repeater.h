#ifndef DIFFERENTIAL_REPEATER_H
#define DIFFERENTIAL_REPEATER_H

#include "Arduino.h" //double check if this is needed

#define DEFAULT_TEMPO 80
#define DEFAULT_LOOP_POINT 15
#define MIN_TEMPO 10
#define MAX_TEMPO 250
#define STEPS 128
#define TRACKS 17
#define POLYPHONY 12
#define COPY_BUFFER 16
#define OFFSET 36

typedef void (*MIDIcallback) (uint8_t note, uint8_t type, uint8_t channel);

//typedef void (*STEPcallback) (uint8_t step);



//using namespace std;

class Engine
{

  public:
    Engine(); //
    void _play(); //
    void pause(); //
    void _stop(); //
    void transport(); //
    void toggleRecord(); //
    void setDivision(); //
    uint8_t getPosition(uint8_t track); //
    void setLoopPoint(int track, uint8_t point); //
    void setTempo(int bpm); //
    void writeNoteOn(uint8_t degree); //
    void writeNoteOff(uint8_t degree); //
    void octaveUp(); //
    void octaveDown(); //
    void scaleUp(); //
    void scaleDown(); //
    void setMidiHandler(MIDIcallback cb); //
    //temp debugging functions
    unsigned long getSixteenth(); //
    unsigned long getTicks(); //
    //void allNotesOff();
    //void playStep();
    //void goForward();
    //void goBackward();
    //void goRandom(); 

  private:
    struct Space
    {
      uint8_t degree:4;
      uint8_t octave:3;
      uint8_t isOn:1;
    };
    struct Note
    {
       
       uint8_t note:7; //only need 7 bits for a midi note
       uint8_t type:1; //on or off works for me
       uint8_t channel;   //see README
    };
    
    struct errorLocation
    {
      uint8_t x;
      uint8_t y;
      uint8_t z;
    };
    
    Space _NoteOn[STEPS][TRACKS][POLYPHONY];
    Note NoteOutbox[192];
    //errorLocation last;
    const uint8_t table[13][12] =
    {      {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19},   //Major Scale
           {0, 2, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19},   //Minor Scale
           {0, 2, 3, 5, 7, 9, 10, 12, 14, 15, 17, 19},   //Dorian
           {0, 1, 3, 5, 7, 8, 10, 12, 13, 15, 17, 19},   //Phyrigian
           {0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19},   //Lydian
           {0, 2, 4, 5, 7, 9, 10, 12, 14, 16, 17, 19},   //Mixolydian
           {0, 1, 3, 5, 6, 8, 10, 12, 13, 15, 17, 18},   //Locrian
           {0, 2, 4, 7, 9, 12, 14, 16, 19, 21, 24, 26},  //Major Pentatonic
           {0, 3, 5, 7, 10, 12, 15, 17, 19, 22, 24, 27}, //Minor Pentatonic
           {0, 3, 5, 6, 7, 10, 12, 15, 17, 18, 19, 22},  //Blues
           {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 24},  //Whole Tone
           {0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16},    //Octatonic
           {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}        //Chromatic
    };
    MIDIcallback midicb;
    uint8_t loopPoint[16];
    uint8_t seqPosition[16];  //position is kind of misleading perhaps a better name
    uint8_t lowFilter;
    uint8_t highFilter;
    int tempo;
    uint8_t octave;
    uint8_t scale;
    uint8_t noteCounter;
    uint8_t trackRecord;
    unsigned long _clock;
    unsigned long sixteenth;
    //unsigned long sixth;
    //unsigned long shuffle;
    unsigned long nextBeat;
    unsigned long nextTick;
    //unsigned long shuffleDivision();
    bool isRunning;
    bool isRecording;
    bool writeEnabled[16];
    bool isForward[16];
    bool isBackward[16];
    bool isRandom[16]; 
    void tick();
    void _step();
    uint8_t quantize();
    uint8_t getNextPosition(uint8_t track);
    void loadOutBox();
    void buildNote(uint8_t degree, uint8_t octave, uint8_t on, uint8_t counter, uint8_t channel);
    void triggerNotes();
};
#endif     
