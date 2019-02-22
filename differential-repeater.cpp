#include "differential-repeater.h"


//Constructor

Engine::Engine()
{ 
  // Initialize global sequence memory
  // degree value of 15 is used to signify an unoccupied space
  // isOn is basically a state change place holder - initialize "false"
  // octave is only relevant when building notes - initialize to zero
  // Note: with this implementation if you do something like change the
  // octave while holding a note and recording the note on/off wont match

  //Note On Data 
  for(uint8_t x = 0; x < STEPS; x++)
  {
    for(uint8_t y = 0; y < TRACKS; y++)
    {
      for(uint8_t z = 0; z < POLYPHONY; z++)
      {
        _NoteOn[x][y][z].degree = 15;
        _NoteOn[x][y][z].octave = 0;
        _NoteOn[x][y][z].isOn = 0;
        
      }
    }
  }

  //Initialize Loop Position and position for all tracks
  //These values can be changed with setLoopPoint function
  //The 17th track is just a copy buffer and doesn't need a loop point
  
  for (uint8_t i = 0; i < TRACKS - 1; i++)
  {
    loopPoint[i] = DEFAULT_LOOP_POINT;
    seqPosition[i] = 0;
    writeEnabled[i] = false;
    isForward[i] = true;
    isBackward[i] = false;
    isRandom[i] = false;
  }
  
  //Initialize Note Outbox arrays
  //These are set to accomodate the possibility of all 16 tracks playing
  //all possibly polyphony at once and also makes it possible to not have
  //to search the entire list for each step

  for (uint8_t i = 0; i < 192; i++)
  {
    NoteOutbox[i].channel = 0;
    NoteOutbox[i].note = 0;
    NoteOutbox[i].type = 0;
  }

  //Set playback and recording variables
   
 
  setTempo(DEFAULT_TEMPO);
 
  //low and high filters will come in later with creation destruction algorithms

  lowFilter = 0;

  highFilter = 127;
 
  octave = 0;

  scale = 0;

  noteCounter = 0;
  
  isRunning = false;

  isRecording = false;

  nextTick = 0;

  nextBeat = 0;

  trackRecord = 0;
  
  /*
  last.xxx = 0;
  last.yyy = 0;
  last.zzz = 0;
  */
  
}

void Engine::transport()
{

  if(!isRunning)
    return;

  unsigned long _now = millis();

  if(_now >= nextTick)
  {
    tick();
    nextTick = _now + _clock;
  }

  if(_now < nextBeat)
    return;

  _step();

  nextBeat = _now + sixteenth;

}

void Engine::_play()
{
  if(isRunning)
    return;

  setTempo(tempo);

  isRunning = true;
  
  return;
}

void Engine::pause()
{
  isRunning = isRunning ? false : true;
  
  return;
}
    
void Engine::_stop()
{
  isRunning = false;

  for(uint8_t i = 0; i < TRACKS - 1; i++)
  {
    seqPosition[i] = 0;
  }

  return;
}

void Engine::setTempo(int bpm)
{

  tempo = bpm;

  if(tempo < MIN_TEMPO)
    tempo = MIN_TEMPO;

  if(tempo > MAX_TEMPO)
    tempo = MAX_TEMPO;

  sixteenth = 60000L / tempo / 4;

  _clock = 60000L / tempo / 24;

  //rip off shuffle implementation from FifteenStep if desirable
  return;
}

void Engine::octaveUp()
{
  if(octave < 7)
  octave++;
  
  return;
}

void Engine::octaveDown()
{
  if(octave > 0)
    octave--;

  return;
}

void Engine::scaleUp()
{

  scale++;
  if(scale > 12)
    scale = 0;

  return;
}

void Engine::scaleDown()
{
  scale--;
  if(scale < 0)
    scale = 12;

  return;
}

void Engine::setDivision()
{
  //later
}

void Engine::setLoopPoint(int track, uint8_t point)
{
  loopPoint[track] = point;
  
  return;
}

void Engine::setMidiHandler(MIDIcallback cb)
{
  midicb = cb;
}
  
void Engine::tick()
{
  //send MIDI clock message
  //usbMIDI.sendRealTime(usbMIDI.Clock);
  return;
}

void Engine::_step()
{
  

  for(uint8_t i = 0; i < TRACKS - 1; i++)
  {
    if(isForward[i])
    {
      seqPosition[i] = seqPosition[i] + 1;
      if(seqPosition[i] > loopPoint[i])
        seqPosition[i] = 0;
    }
  

   if(isBackward[i])
    {
      seqPosition[i] = seqPosition[i] - 1;
      if(seqPosition[i] < 0)
        seqPosition[i] = loopPoint[i];
    }

    if(isRandom[i])
      seqPosition[i] = random(millis()) % loopPoint[i];
  }

  //dump the outbox
  triggerNotes();

  //perform constructive algorithms
  //construct();

  //perform destructive algorithms
  //destruct();

  //load the next step in the outbox
  loadOutBox();  

}

void Engine::writeNoteOn(uint8_t degree)
{
  //sound the scaled note
  
  midicb(table[scale][degree] + octave * 12 + OFFSET, 1, 1);

  //don't write if not recording
  if(!isRecording)
    return;
    
  //lock in a step to write to
  uint8_t quantizedPosition = quantize();
  
  //find an open space then stop looking for one
  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
    if(_NoteOn[quantizedPosition][trackRecord][i].degree == 15)
    {
      _NoteOn[quantizedPosition][trackRecord][i].degree = degree;
      _NoteOn[quantizedPosition][trackRecord][i].octave = octave;
      _NoteOn[quantizedPosition][trackRecord][i].isOn = 1;
      
      //store location incase of note off error
      /*
      last.x = quantizedPosition;
      last.y = trackRecord;
      last.z = i;
      */
      break;
    }
  }
}

void Engine::writeNoteOff(uint8_t degree)
{
  //off the scaled note
  
  midicb(table[scale][degree] + octave * 12 + OFFSET, 0, 1);

  //don't record if not recording
  if(!isRecording)
    return;
    
  //lock in a step to write to
  uint8_t quantizedPosition = quantize();
  
  //find an open space then stop looking for one
  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
    if(_NoteOn[quantizedPosition][trackRecord][i].degree == 15)
    {
      _NoteOn[quantizedPosition][trackRecord][i].degree = degree;
      _NoteOn[quantizedPosition][trackRecord][i].octave = octave;
      _NoteOn[quantizedPosition][trackRecord][i].isOn = 0;
      break;
    }
    /*
    //woops there are no open slots for a note off
    if(i == POLYPHONY - 1)
    {
      //reinitialize last note to avoid hanging notes
      _NoteOn[last.x][last.y][last.z].degree = 15;
      
      //uncomment to autosearch for next available space for note off

      // COMMENT THIS OUT
      for(uint8_t next = quantizedPosition; next <= loopPoint[trackRecord]; next++)
      {
        _NoteOn[quantizedPosition + next][trackRecord][i].degree = degree;
        _NoteOn[quantizedPosition + next][trackRecord][i].octave = octave;
        _NoteOn[quantizedPosition + next][trackRecord][i].isOn = 0;
        break;
      }
     
   }
   */ 
  }
}
    
void Engine::loadOutBox()
{
  //reset the note counter
  noteCounter = 0;

  for(uint8_t i = 0; i < TRACKS - 1; i++)
  {
    for(uint8_t j = 0; j < POLYPHONY; j++)
    {
      if(_NoteOn[getNextPosition(i)][i][j].degree != 15)
      {
        buildNote(
          _NoteOn[getNextPosition(i)][i][j].degree,
          _NoteOn[getNextPosition(i)][i][j].octave,
          _NoteOn[getNextPosition(i)][i][j].isOn,
          noteCounter,
          i);

        //count the notes for the trigger
        noteCounter++;
      }
    }
  }
}

void Engine::triggerNotes()
{
  for(uint8_t i = 0; i < noteCounter; i++)
  {
    midicb(NoteOutbox[i].note, NoteOutbox[i].type, NoteOutbox[i].channel);
  }
}

void Engine::buildNote(uint8_t degree, uint8_t octave, uint8_t on, uint8_t counter, uint8_t channel)
{
    NoteOutbox[counter].channel = channel + 1;
    NoteOutbox[counter].note = table[scale][degree] + octave * 12 + OFFSET;
    NoteOutbox[counter].type = on; 
}

uint8_t Engine::quantize()
{
  unsigned long writeNow = millis();

  unsigned long thirty_second = sixteenth / 2;

  if(writeNow <= (nextBeat - thirty_second))
    return seqPosition[trackRecord];

  if((seqPosition[trackRecord] + 1) >= loopPoint[trackRecord])
    return 0;

  return seqPosition[trackRecord] + 1;
}

void Engine::toggleRecord()
{
  isRecording = isRecording ? false : true;
  
}

uint8_t Engine::getPosition(uint8_t track)
{
  return seqPosition[0];
}

uint8_t Engine::getNextPosition(uint8_t track)
{
  if(1 + seqPosition[track] > loopPoint[track])
    return 0;

  return (1 + seqPosition[track]);
}

unsigned long Engine::getSixteenth()
{
  return sixteenth;
}

unsigned long Engine::getTicks()
{
  return _clock;
}

 
