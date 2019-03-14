#include "differential-repeater.h"


//Constructor

Engine::Engine()
{ 

}

void Engine::_begin()
{
  // Initialize global sequence memory
  // degree value of 15 is used to signify an unoccupied space
  // isOn is basically a note on/off place holder - initialize "false"
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
    startPoint[i] = 0;
    seqPosition[i] = 0;
    trackScale[i] = 0;
    writeEnabled[i] = false;
    seqDirection[i] = 1;
    isBackward[i] = false;
    isRandom[i] = false;
  }
  
  //Initialize Note Outbox arrays
  //These are set to accomodate the possibility of all 16 tracks playing
  //all possibly polyphony at once and also makes it possible to not have
  //to search the entire list for each step
  for (uint8_t i = 0; i < TRACKS; i++)
  {
    for (uint8_t j = 0; j < 128; j++)
    {
      NoteOutbox[i].note = 0;
      NoteOutbox[i].type = 0;
      NoteOutbox[i].channel = 0;
    }
  }

  //Set playback and recording variables
   
 
  setTempo(DEFAULT_TEMPO);
 
  //low and high filters will come in later with creation destruction algorithms

  lowFilter = 0;

  highFilter = 127;
 
  octave = 0;

  scale = 0;

  key = OFFSET;

  loaded = 0;

  previousKeys = 0;
  
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
  
  //perform constructive algorithms
  //construct();

  //perform destructive algorithms
  //destruct();

  //load the next step in the outbox
  if(!loaded)
    loadOutBox();
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
    seqPosition[i] = startPoint[i];
  }
  allNotesOff();

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

void Engine::keyUp()
{
  if(key < 48)
    key++;

  return;
}

void Engine::keyDown()
{
  if(key > 0)
    key--;

  return;
}

void Engine::setDivision()
{
  //later
}

void Engine::setLoopPoint(uint8_t track, uint8_t point)
{
  loopPoint[track] = point;
  
  return;
}

void Engine::setStartPoint(uint8_t track, uint8_t point)
{
  startPoint[track] = point;
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
    if(seqDirection[i] == 0x1)
    {
      uint8_t next = seqPosition[i] + 1;
      if(next > loopPoint[i])
        seqPosition[i] = startPoint[i];
      else if (next >= STEPS)
        seqPosition[i] = 0;
      else
        seqPosition[i] = next;

     }
  

    if(seqDirection[i] == 0x2)
    {
      int next = seqPosition[i] - 1;
      if(next < startPoint[i])
        seqPosition[i] = loopPoint[i];
     // else if (next < 0)
     //   seqPosition[i] = STEPS - 1;
      else
        seqPosition[i] = next;
    }

    if(seqDirection[i]== 0x3)
      seqPosition[i] = random(startPoint[i], loopPoint[i]);
  }

  triggerNotes();
  loaded = false;
  //dump the outbox
  

}
void Engine::setDirection(uint8_t track, uint8_t direc)
{
  seqDirection[track] = direc;

  if(seqDirection[track] == 0x2)
    flip(track);
}

void Engine::flip(uint8_t track)
{
  for(uint8_t i = 0; i < STEPS; i++)
  {
   for(uint8_t j = 0; j < POLYPHONY; j++)
   {
    if(_NoteOn[i][track][j].degree != 15)
      _NoteOn[i][track][j].isOn = !_NoteOn[i][track][j].isOn;
   }
  }
}

void Engine::writeNoteOn(uint8_t degree)
{
  //sound the scaled note
  
  midicb(table[trackScale[trackRecord]][degree] + octave * 12 + OFFSET, 1, 1);

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
  
  midicb(table[trackScale[trackRecord]][degree] + octave * 12 + OFFSET, 0, 1);

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
void Engine::writeNote(uint16_t keyReg)
{
  /*
  //lock in a step to write to
  uint8_t quantizedPosition = quantize();
  
    for (uint8_t i = 0; i < 12; i++)
    {
      if(keyReg & (1 << i) && !(previousKeys & (1 << i)) )
      {
        uint8_t val = buildNote(table[trackScale[trackRecord]][i], octave);
        
        midicb(val, 1, trackRecord);
        
        if(isRecording && isRunning && !_NoteOn[quantizedPosition][trackRecord][i].isOn)
        {
          _NoteOn[quantizedPosition][trackRecord][i].octave = octave;
          _NoteOn[quantizedPosition][trackRecord][i].isOn = 1;
        }
      }
      
      if (!( keyReg & (1 << i) ) && ( previousKeys & (1 << i)) )
      { 
        uint8_t valOff = buildNote(table[trackScale[trackRecord]][i], octave);
        midicb(valOff, 0, trackRecord);

        if(isRecording && isRunning && !_NoteOn[quantizedPosition][trackRecord][i].isOn)
        {
          _NoteOn[quantizedPosition][trackRecord][i].octave = _NoteOn[quantizedPosition - 1][trackRecord][i].octave;
          _NoteOn[quantizedPosition][trackRecord][i].isOn = 0;
        }
      }
    }

  previousKeys = keyReg;
  */
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
  loaded = true;
}
/*    
void Engine::loadOutBox()
{
  uint8_t thisStep;
  uint8_t load;
  
  for(uint8_t i = 0; i < TRACKS - 1; i++)
  {
    thisStep = getNextPosition(i);
    
    for(uint8_t j = 0; j < POLYPHONY; j++)
    {
      load = buildNote(table[trackScale[i]][j], _NoteOn[thisStep][i][j].octave);
      
      if(_NoteOn[thisStep][i][j].isOn != NoteOutbox[i][load].state)
      {
         NoteOutbox[i][load].state = _NoteOn[thisStep][i][j].isOn;
         NoteOutbox[i][load].changed = true;
      }
    }
  }
  loaded = true;
}
*/

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
    NoteOutbox[counter].note = table[trackScale[channel]][degree] + octave * 12 + OFFSET;
    NoteOutbox[counter].type = on; 
}

/*
void Engine::triggerNotes()
{ 
  
  for(uint8_t i = 0; i < 16; i++)
  {
    for(uint8_t j = 0; j < 128; j++)
    {
      if(NoteOutbox[i][j].changed)
      {
        midicb(j, NoteOutbox[i][j].state, i);
        NoteOutbox[i][j].changed = false;
      }
    }
  }
}

uint8_t Engine::buildNote(uint8_t degree, uint8_t octave)
{
    uint8_t built = (degree + octave * 12 + key);
    return built;
}
*/

uint8_t Engine::quantize()
{
  unsigned long writeNow = millis();

  unsigned long thirty_second = sixteenth / 2;

  if(writeNow <= (nextBeat - thirty_second))
    return seqPosition[trackRecord];

  if((seqPosition[trackRecord] + 1) >= loopPoint[trackRecord])
    return startPoint[trackRecord];

  return seqPosition[trackRecord] + 1;
}

void Engine::selectTrackRecord(uint8_t track)
{
  trackRecord = track;
}

void Engine::toggleRecord()
{
  isRecording = isRecording ? false : true;
  
}

uint8_t Engine::getPosition(uint8_t track)
{
  return seqPosition[track];
}

uint8_t Engine::getWrite(uint8_t track)
{
  return writeEnabled[track];
}

uint8_t Engine::getRunning()
{
  return isRunning;
}
uint8_t Engine::getNextPosition(uint8_t track)
{
  if(1 + seqPosition[track] > loopPoint[track])
    return startPoint[track];

  return (1 + seqPosition[track]);
}

unsigned long Engine::getSixteenth()
{
  return sixteenth;
}

uint8_t Engine::getTrackRecord()
{
  return trackRecord;
}

unsigned long Engine::getTicks()
{
  return _clock;
}

void Engine::trackAllNotesOff(uint8_t track)
{
  for(uint8_t i = 0; i < 128; i++)
  {
    midicb(i, 0, track);
  }
}

void Engine::allNotesOff()
{
  for(uint8_t j = 0; j < 16; j++)
  {
    for(uint8_t i = 0; i < 128; i++)
    {
      midicb(i, 0, j);
    }
  }
}

void Engine::toggleWriteEnabled(uint8_t track)
{
  writeEnabled[track] = writeEnabled[track] ? false : true;
  
}

void Engine::setTrackScale(uint8_t scale)
{
  if(scale >= 13)
    return;
  //pause? 
  trackScale[trackRecord] = scale;
  trackAllNotesOff(trackRecord);
  //play?
  
}

void Engine::construction(uint8_t algorithm)
{
  
  for (uint8_t i = 0; i < TRACKS - 1; i++)
  {
    if(writeEnabled[i])
    {
      switch(algorithm)
      {
        case 1:
        //shift scope up
        startPoint[i] += 1;
        loopPoint[i] += 1;
        
        case 2:
        //basic fugue
        for(uint8_t j = 0; j < loopPoint[i]; j++)
        {
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            if(_NoteOn[j][i][k].degree != 15)
            {
              if((_NoteOn[j][i][k].degree + 4) > 11)
              {
                _NoteOn[j + loopPoint[i]][i][POLYPHONY - (k + 1)].degree = (_NoteOn[j][i][k].degree + 4) % 8;
                _NoteOn[j + loopPoint[i]][i][POLYPHONY - (k + 1)].octave = _NoteOn[j][i][k].octave;
                _NoteOn[j + loopPoint[i]][i][POLYPHONY - (k + 1)].isOn = _NoteOn[j][i][k].isOn;
              } else {
                _NoteOn[j + loopPoint[i]][i][POLYPHONY - (k + 1)].degree = _NoteOn[j][i][k].degree + 4;
                _NoteOn[j + loopPoint[i]][i][POLYPHONY - (k + 1)].octave = _NoteOn[j][i][k].octave;
                _NoteOn[j + loopPoint[i]][i][POLYPHONY - (k + 1)].isOn = _NoteOn[j][i][k].isOn;
              }
            }
          }
        }
        loopPoint[i] = loopPoint[i] * 2;
        
        case 3:
        //fugue shorter (but how to quantize? only accept 8th notes?
        
        case 4:
        //add random note
        
        uint8_t newStep;
        uint8_t newPoly;
        
        while(1)
        {
          newStep = random(startPoint[i], loopPoint[i]);
          newPoly = random(POLYPHONY);
          
          if(_NoteOn[newStep][i][newPoly].degree == 15)
          {
            
            uint8_t newNote = random(12);
            _NoteOn[newStep][i][newPoly].degree = newNote;
            _NoteOn[newStep][i][newPoly].octave = octave;
            _NoteOn[newStep][i][newPoly].octave = 1;

            for(uint8_t j = 0; j < (loopPoint[i] - startPoint[i]); j++)
            {
              for(uint8_t k = 0; k < POLYPHONY; k++)
              {
                 if( _NoteOn[(newStep + j) % loopPoint[i]][i][k].degree != 15)
                 {
                   uint8_t val = (newStep + j) % loopPoint[i];
                   _NoteOn[val][i][newPoly].degree = newNote;
                   _NoteOn[val][i][newPoly].octave = octave;
                   _NoteOn[val][i][newPoly].octave = 0;
                   return;
                 }
              }              
            }
            
          }
        }
        
        case 5:
        //add backwards fugue longer
        for(uint8_t j = startPoint[i]; j < loopPoint[i]; j++)
        {
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            if(_NoteOn[j][i][k].degree != 15)
            {
              if((_NoteOn[j][i][k].degree + 4) > 11) //change 4 to a DEFINE and create a fugue harmony variable that can be changed
              {
                _NoteOn[loopPoint[i]*2 - j][i][POLYPHONY - (k + 1)].degree = (_NoteOn[j][i][k].degree + 4) % 8;
                _NoteOn[loopPoint[i]*2 - j][i][POLYPHONY - (k + 1)].octave = _NoteOn[j][i][k].octave;
                _NoteOn[loopPoint[i]*2 - j][i][POLYPHONY - (k + 1)].isOn = _NoteOn[j][i][k].isOn;
              } else {
                _NoteOn[loopPoint[i]*2 - j][i][POLYPHONY - (k + 1)].degree = _NoteOn[j][i][k].degree + 4;
                _NoteOn[loopPoint[i]*2 - j][i][POLYPHONY - (k + 1)].octave = _NoteOn[j][i][k].octave;
                _NoteOn[loopPoint[i]*2 - j][i][POLYPHONY - (k + 1)].isOn = _NoteOn[j][i][k].isOn;

              }
            }
          }
        }
        loopPoint[i] = loopPoint[i] * 2;
        case 6:
        //add backwards fugue shorter
        case 7:
        //shift scope down
        
        //actually have to re-write step algorithms to cross from max step to 0 and back
        
        case 8:
        //widen scope

        //also the case here
        
        case 9:
        //focus scope
        if(startPoint[i] - loopPoint[i] >= 4)
        {
          startPoint[i]++;
          loopPoint[i]--;
        }
        
        
        case 10:
        //repeat
        case 11:
        //mirror
        
        for(uint8_t j = startPoint[i]; j < loopPoint[i]; j++)
        {
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
           
          }
        }
        case 12:
        //add octave double
        case 13:
        //add harmony (lfo determines interval?)
        case 14:
        //add fugue octave
        case 15:
        //add fugue harmony
        case 16:
        //close range equally
        case 17:
        //raise lowfilter
        case 18:
        //lower highfilter
        case 19:
        //midi delay
        default:
           continue;
      }
    }
  }

}

void Engine::destruction(uint8_t algorithm)
{
  switch(algorithm)
  {
    case 1:
    //erase random note
    case 2:
    //erase lowest notes
    case 3:
    //erase highest notes
    case 4:
    //double space
    case 5:
    //tripple space
    case 6:
    //modulator spaceout 1
    case 7:
    //crunch down
    case 8:
    //quantize to 8th notes
    case 9:
    //quantize to 12th notes
    case 10:
    //invert pitches
    case 11:
    //double length (legato)
    case 12:
    //halve length (legato)
    case 13:
    //
    default:
    break;
  }
}

uint8_t Engine::printScales(uint8_t x, uint8_t y)
{
  return table[x][y];
}

 
