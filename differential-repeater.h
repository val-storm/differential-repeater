#ifndef DIFFERENTIAL_REPEATER_H
#define DIFFERENTIAL_REPEATER_H

#include "Arduino.h" //double check if this is needed

#define DEFAULT_TEMPO 120
#define DEFAULT_LOOP_POINT 15
#define MIN_TEMPO 10
#define MAX_TEMPO 250
#define STEPS 256
#define TRACKS 16
#define POLYPHONY 16
#define COPY_BUFFER 16
#define OFFSET 36
#define DEFAULT_HARMONY 4

typedef void (*MIDIcallback) (uint8_t note, uint8_t type, uint8_t channel);

//typedef void (*STEPcallback) (uint8_t step);

//current scale list not the most fun or comprehensive, would like to implement scala tunings
//which to my understanding would have specific 14 bit pitchbend values associated with midi
//notes. Probably a summer project.

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
  

//using namespace std;

class Engine
{

  public:
    Engine();
    void _begin();
    void _play();
    void pause();
    void _stop();
    void transport();
    void toggleRecord();
    void setDivision();
    void changeDirection(uint8_t track);
    uint8_t getPosition(uint8_t track);
    uint8_t getWrite(uint8_t track);
    uint8_t getRunning();
    bool getRecording();
    void initializeTrack();
    void setLoopPoint(uint8_t point);
    void setStartPoint(uint8_t track, uint8_t point);
    void pageUp(uint8_t track);
    void pageDown(uint8_t track);
    void setTempo(int bpm);
    void writeNoteOn(uint8_t degree);
    void writeNoteOff(uint8_t degree);
    void undoLastNote();
    void octaveUp();
    void octaveDown();
    void keyUp();
    void keyDown();
    void muteTrack();
    void muteTrack(uint8_t track);
    void muteAll();
    void toggleWriteEnabled(uint8_t track);
    void setTrackScale(uint8_t scale);
    void setMidiHandler(MIDIcallback cb);
    //temp debugging functions
    uint8_t getTrackRecord();
    void selectTrackRecord(uint8_t track);
    unsigned long getSixteenth();
    unsigned long getTicks();
    void construction(uint8_t algorithm);
    void destruction(uint8_t algorithm);
    void trackAllNotesOff(uint8_t track);
    void allNotesOff();
    uint8_t printScales(uint8_t x, uint8_t y);
    uint8_t loopPoint[16];
    uint8_t seqPosition[16];  //position is kind of misleading perhaps a better name
    uint8_t startPoint[16];
    bool writeEnabled[16];
    uint8_t seqDirection[16];
    
    //bool isBackward[16];
    //bool isRandom[16];
    uint8_t trackScale[16];
    //void playStep();
    //void goForward();
    //void goBackward();
    //void goRandom(); 

  private:
    struct Space
    { 
      uint8_t duration:8;
      uint8_t degree:4;
      uint8_t octave:3;
      uint8_t isOn:1;
      Space(): degree(15)
      {
      }
      
      
    };
    struct Note
    {
       uint8_t duration;
       uint8_t note:7; //only need 7 bits for a midi note
       uint8_t type:1; //on or off works for me
       Note(): type(0) {}
        //see README
       
    };
    struct Timer
    {
      unsigned long startTime;
      uint8_t  startStep;
    };
    
    Space _NoteOn[STEPS][17][POLYPHONY];
   
    Note NoteOutbox[TRACKS][POLYPHONY];
    
    Timer polycounter[POLYPHONY];

    Space * last = NULL;

    bool keysOn[16][128] = { { 0 , 0 } };
    bool trackMute[TRACKS] = { 0 };
    
    MIDIcallback midicb;
   
    uint8_t lowFilter;
    uint8_t highFilter;
    int tempo;
    uint8_t key[TRACKS];
    uint8_t octave;
    uint8_t scale;
    uint8_t polyIndex[TRACKS] = { 0 };
    //unsigned long division[TRACKS];
    uint8_t trackRecord;
    uint16_t previousKeys;
    unsigned long _clock;
    unsigned long sixteenth;
    unsigned long beatDiv_8;
    unsigned long beatDiv_8_triplets;
    unsigned long beatDiv_32;
    unsigned long * division[16];
    //unsigned long sixth;
    //unsigned long shuffle;
    //unsigned long shuffleDivision();
    unsigned long nextBeat[TRACKS];
    unsigned long nextTick;
    bool isRunning;
    bool isRecording;
    uint8_t noteCounter;
    uint8_t lastWrite;
    void tick();
    void _step(uint8_t track);
    uint8_t quantize(uint8_t track);
    uint8_t getNextPosition(uint8_t track);
    void durationTracker(uint8_t track);
    void triggerNotes(uint8_t track);
};
#endif     
