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
  /*
  //Note On Data 
  for(uint8_t x = 0; x < STEPS; x++)
  {
    for(uint8_t y = 0; y < 17; y++)
    {
      for(uint8_t z = 0; z < POLYPHONY; z++)
      {
         
        _NoteOn[x][y][z].degree = 15;
       // _NoteOn[x][y][z].octave = 0;
       // _NoteOn[x][y][z].isOn = 0;
       // _NoteOn[x][y][z].duration = 0;
        delay(1);
      }
    }
  }
  */
  
  //Initialize Loop Position and position for all tracks
  //These values can be changed with setLoopPoint function
  //The 17th track is just a copy buffer and doesn't need a loop point
  
  for (uint8_t i = 0; i < TRACKS; i++)
  {
    loopPoint[i] = DEFAULT_LOOP_POINT;
  }

  for (uint8_t i = 0; i < TRACKS; i++)
  {
    startPoint[i] = 0;
  }
  for (uint8_t i = 0; i < TRACKS; i++)
  {
    seqPosition[i] = 0;
  }
  for (uint8_t i = 0; i < TRACKS; i++)
  {
    trackScale[i] = 0; 
  }
   for (uint8_t i = 0; i < TRACKS; i++)
  {
    loaded[i] = 0;
  }
   for (uint8_t i = 0; i < TRACKS; i++)
  {
    nextBeat[i] = 0;
  }
   for (uint8_t i = 0; i < TRACKS; i++)
  {
    key[i] = OFFSET;
  }
   for (uint8_t i = 0; i < TRACKS; i++)
  {
    writeEnabled[i] = false;
  }
   for (uint8_t i = 0; i < TRACKS; i++)
  {
    seqDirection[i] = 0;
  }
  for (uint8_t i = 0; i < TRACKS; i++)
  {
    polycounter[i].startTime = 0;
    polycounter[i].startStep = 0;
  }
  //Initialize Note Outbox arrays
  //These are set to accomodate the possibility of all 16 tracks playing
  //all possibly polyphony at once and also makes it possible to not have
  //to search the entire list for each step
  for (uint8_t i = 0; i < TRACKS; i++)
  {
    for (uint8_t j = 0; j < POLYPHONY; j++)
    {
      NoteOutbox[i][j].note = 0;
      NoteOutbox[i][j].type = 0;
      //NoteOutbox[i][j].channel = 0;
      NoteOutbox[i][j].duration = 0;
    }
  }

  //Set playback and recording variables
  setTempo(DEFAULT_TEMPO);
  
  for (uint8_t i = 0; i < TRACKS; i++)
  {
    division[i] = sixteenth;
  }
 

  //low and high filters will come in later with creation destruction algorithms

  lowFilter = 0;

  highFilter = 127;
 
  octave = 0;

  scale = 0;

  //key = OFFSET;


  previousKeys = 0;
  
  isRunning = false;

  isRecording = false;

  nextTick = 0;

  //nextBeat = 0;

  trackRecord = 0;
  

  
  
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

  for(uint8_t i = 0; i < TRACKS; i++)
  {
    if(_now < nextBeat[i])
      continue;

    _step(i);

    nextBeat[i] = _now + division[i];
  }

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
  //allNotesOff();
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

void Engine::_step(uint8_t track)
{

  durationTracker(track);
  
  if(seqDirection[track] == 0)
  {
    uint8_t next = seqPosition[track] + 1;
    if(next > loopPoint[track])
      seqPosition[track] = startPoint[track];
    else if (next >= STEPS)
      seqPosition[track] = 0;
    else
      seqPosition[track] = next;

   }


  if(seqDirection[track] == 1)
  {
    int next = seqPosition[track] - 1;
    if(next < startPoint[track])
      seqPosition[track] = loopPoint[track];
   // else if (next < 0)
   //   seqPosition[track] = STEPS - 1;
    else
      seqPosition[track] = next;
  }

  if(seqDirection[track] == 2)
    seqPosition[track] = random(startPoint[track], loopPoint[track]);
  
  
  triggerNotes(track);
  
  loaded[track] = false;
  

}


void Engine::flip(uint8_t track)
{
  /*
  for(uint8_t i = 0; i < STEPS; i++)
  {
   for(uint8_t j = 0; j < POLYPHONY; j++)
   {
    if(_NoteOn[i][track][j].degree != 15)
      _NoteOn[i][track][j].isOn = !_NoteOn[i][track][j].isOn;
   }
  }
  */
}

void Engine::writeNoteOn(uint8_t degree)
{ 
  
  
  //sound the scaled note
  
  midicb(table[trackScale[trackRecord]][degree] + octave * 12 + key[trackRecord], 1, trackRecord + 1);

  //don't write if not recording
  if(!isRecording)
    return;
    
  //lock in a step to write to
  uint8_t quantizedPosition = quantize(trackRecord);
  
  polycounter[degree].startTime = millis();
  polycounter[degree].startStep = quantizedPosition;
  
}

void Engine::writeNoteOff(uint8_t degree)
{
  //off the scaled note
  
  midicb(table[trackScale[trackRecord]][degree] + octave * 12 + key[trackRecord], 0, trackRecord + 1);

  //still don't record if not recording
  if(!isRecording)
    return;
    
  uint8_t writeStep = polycounter[degree].startStep;
  
  uint8_t duration = (millis() - polycounter[degree].startTime) / division[trackRecord];

  if(duration < 1)
    duration = 1;
    
  //find an open 'space' then stop looking for one
  
  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
    if(_NoteOn[writeStep][trackRecord][i].degree == 15)
    {
      _NoteOn[writeStep][trackRecord][i].duration = duration;
      _NoteOn[writeStep][trackRecord][i].degree = degree;
      _NoteOn[writeStep][trackRecord][i].octave = octave;
      //Serial.println("Hi");
      break;
    }
    
  }
}
// to be deleted
void Engine::readKeys(uint16_t keys)
{
  if(!isRecording && !isRunning)
    return;
    
  uint8_t quant = quantize(trackRecord);
 
  if(quant == lastWrite)
    return;
    
  
  
  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
     if( (keys & (1 << i)) >> i )
      _NoteOn[quant][trackRecord][i].isOn = ((keys & (1 << i)) >> i); 
  }
  lastWrite = quant;
}
//tbd
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
//tbd
void Engine::loadOutBox(uint8_t track)
{
  //reset the note counter
  noteCounter = 0;
  uint8_t next = getNextPosition(track);
  
  for(uint8_t j = 0; j < POLYPHONY; j++)
  {
    if(_NoteOn[next][track][j].degree != 15)
    {
      buildNote(
          _NoteOn[next][track][j].degree,
          _NoteOn[next][track][j].octave,
          _NoteOn[next][track][j].duration,
          noteCounter,
          track);

        //count the notes for the trigger
        noteCounter++;
     }
  }
  loaded[track] = true;
}

void Engine::durationTracker(uint8_t track)
{
  //count down durations, send note off messages if on last step of duration
  
  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
    
    if(NoteOutbox[track][i].duration == 1)
      midicb(NoteOutbox[track][i].note, 0, track + 1);
      keysOn[track][NoteOutbox[track][i].note] = false;
    
    if(NoteOutbox[track][i].duration > 0)
      NoteOutbox[track][i].duration = NoteOutbox[track][i].duration - 1;

  }

 //pack not yet elapsed notes down and keep track of index for new notes
  
  uint8_t counter = 0;
  
  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
    if(NoteOutbox[track][i].duration > 0)
    {
      NoteOutbox[track][counter].duration = NoteOutbox[track][i].duration;
      NoteOutbox[track][counter].note = NoteOutbox[track][i].note;
      counter++;
    }
  }

  polyIndex[track] = counter;
}

void Engine::triggerNotes(uint8_t track)
{
  uint8_t midiNote;
  uint8_t degree;

  //iterate over 'spaces' for initiated data
  
  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
    degree = _NoteOn[seqPosition[i]][track][i].degree;

    //just incase keep looking
    if(degree == 15)
      continue;

    //space has note data, scale the data
    midiNote = table[trackScale[track]][degree] + _NoteOn[seqPosition[i]][track][i].octave * 12 + key[track];

    //better check the "virtual key board" to see if the note is already sounding on this track
    if(keysOn[track][midiNote])
      continue;
    
    //all good, so, sound the note! *MIDI doesn't have track zero*
    midicb(midiNote, 1, (track + 1));

    //update keyboard
    keysOn[track][midiNote] = true;

    //update polyphony management array
    
    NoteOutbox[track][polyIndex[track]].duration =  _NoteOn[seqPosition[i]][track][i].duration;
    NoteOutbox[track][polyIndex[track]].note =  midiNote;

    //shift the index to accept the next note
    polyIndex[track] = polyIndex[track] + 1;
   }
    
}
//tbd
void Engine::buildNote(uint8_t degree, uint8_t octave, uint8_t duration, uint8_t counter, uint8_t track)
{ 
  //for(uint8_t i = 0; i < 16; i++)
  //{ 
    if(NoteOutbox[track][degree].duration == 0)
    {
      NoteOutbox[track][degree].duration = duration;  
      NoteOutbox[track][degree].note = table[trackScale[track]][degree] + octave * 12 + OFFSET;
      NoteOutbox[track][degree].type = 1;
      //break;
    }
 // }
}

uint8_t Engine::quantize(uint8_t track)
{
  unsigned long writeNow = millis();

  unsigned long thirty_second = sixteenth / 2;

  if(writeNow <= (nextBeat[track] - thirty_second))
    return seqPosition[trackRecord];

  if((seqPosition[trackRecord] + 1) >= loopPoint[trackRecord])
    return startPoint[trackRecord];

  return seqPosition[trackRecord] + 1;
}

/*******************************************
*  INTERFACE AND FEEDBACK METHODS
********************************************/

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

bool Engine::getRecording()
{
  return isRecording;
}

uint8_t Engine::getNextPosition(uint8_t track)
{
  if(seqDirection[track] == 0x1)
  {
    uint8_t next = seqPosition[track] + 1;
    if(next > loopPoint[track])
      return startPoint[track];
    else if (next >= STEPS)
      return 0;
    else
      return next;

   }


  if(seqDirection[track] == 0x2)
  {
    int next = seqPosition[track] - 1;
    if(next < startPoint[track])
      return loopPoint[track];
    else if (next < 0 && startPoint[track] != 0)
      return (STEPS - 1);
    else
      return next;
  }

  if(seqDirection[track]== 0x3)
    return random(startPoint[track], loopPoint[track]);

  return 0;
}

void Engine::changeDirection(uint8_t track)
{
  seqDirection[track] = (seqDirection[track] + 1) % 3 ;

}

unsigned long Engine::getSixteenth()
{
  return sixteenth;
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
  if(key[trackRecord] < 48)
    key[trackRecord] = key[trackRecord] + 1;

  return;
}

void Engine::keyDown()
{
  if(key[trackRecord] > 0)
    key[trackRecord] = key[trackRecord] - 1;

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
     // Serial.print(i);
    }
    //Serial.println(j);
  }
}

void Engine::toggleWriteEnabled(uint8_t track)
{
  writeEnabled[track] = writeEnabled[track] ? false : true;
  
}

void Engine::setTrackScale(uint8_t scale)
{
  if(scale > 12)
    return;
  //pause? 
  trackScale[trackRecord] = scale;
  trackAllNotesOff(trackRecord);
  //play?
  
}
/*********************************************
*  CONSTRUCTION ALGORITHMS
*  
*  Which algorithm is set by encoder and sent
*  here to operate on all tracks write enabled
*  
**********************************************/

void Engine::construction(uint8_t algorithm)
{
  /*
  for (uint8_t i = 0; i < TRACKS - 1; i++)
  {
    if(writeEnabled[i])
    {
      switch(algorithm)
      {
        case 1:
        //shift scope up "page up"
        startPoint[i] = (startPoint[i] + 16) % STEPS;
        loopPoint[i] = (loopPoint[i] + 16) % STEPS;
        
        case 2:
        //basic fugue
        uint8_t loc = (loopPoint[i] - startPoint[i]) * 2 + 1;
        
        for(uint8_t j = startPoint[i]; j < loopPoint[i]; j++)
        {
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            if(_NoteOn[j][i][k].degree != 15)
            {
              if((_NoteOn[j][i][k].degree + 4) > 11)
              {
                _NoteOn[j + loc][i][k].duration = _NoteOn[j][i][k].duration;
                _NoteOn[j + loc][i][k].degree = (_NoteOn[j][i][k].degree + 4) % 8;
                _NoteOn[j + loc][i][k].octave = _NoteOn[j][i][k].octave;
                
              } else {
                _NoteOn[j + loc][i][k].duration = _NoteOn[j][i][k].duration;
                _NoteOn[j + loc][i][k].degree = _NoteOn[j][i][k].degree + 4;
                _NoteOn[j + loc][i][k].octave = _NoteOn[j][i][k].octave;
                
              }
            }
          }
        }
        loopPoint[i] = loc;
        
        case 3:
        //retrograde inversion
        
        
        case 4:
        //add random note

        bool complete = false;
        uint8_t newStep;
        uint8_t newPoly;
        
        while()
        {
          newStep = random(startPoint[i], loopPoint[i]);
          newPoly = random(POLYPHONY);
          
          if(_NoteOn[newStep][i][newPoly].degree == 15)
          {
            
            uint8_t newNote = random(12);
            _NoteOn[newStep][i][newPoly].degree = newNote;
            _NoteOn[newStep][i][newPoly].octave = octave;
            _NoteOn[newStep][i][newPoly].duration = millis() % 3;

                         
           }
            
         }
        
        
        case 5:
        
        //retrograde fugue
        
        uint8_t loc = (loopPoint[i] - startPoint[i]) * 2 + 1;
        
        for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
        {
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            if(_NoteOn[j][i][k].degree != 15)
            {
              if((_NoteOn[j][i][k].degree + 4) > 11) //change 4 to a DEFINE and create a fugue harmony variable that can be changed
              {
                _NoteOn[loc - j][i][k].degree = (_NoteOn[j][i][k].degree + 4) % 8;
                _NoteOn[loc - j][i][k].octave = _NoteOn[j][i][k].octave + 1;
                _NoteOn[loc - j][i][k].duration = _NoteOn[j][i][k].duration;
              } else {
                _NoteOn[loc - j][i][k].degree = _NoteOn[j][i][k].degree + 4;
                _NoteOn[loc - j][i][k].octave = _NoteOn[j][i][k].octave;
                _NoteOn[loc - j][i][k].duration = _NoteOn[j][i][k].duration;
              }
            }
          }
        }
        loopPoint[i] = loc;
        
        case 6:
        
        //the "maybe copy"
        
        uint8_t maybe;
        uint8_t nextPage = loopPoint[i] + 1;

        for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
        {
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            maybe = random() % 3;
            
            if(_NoteOn[j][i][k].degree != 15 && maybe)
            {
              _NoteOn[j + nextPage][i][k].duration = _NoteOn[j][i][k].duration;
              _NoteOn[j + nextPage][i][k].degree = _NoteOn[j][i][k].degree;
              _NoteOn[j + nextPage][i][k].octave = _NoteOn[j][i][k].octave;           
            }
          }
        }
        loopPoint[i] = loopPoint[i] * 2 + 1;

        
        case 7:
        //shift scope down

        //might not need scope
        
        //actually have to re-write step algorithms to cross from max step to 0 and back
        
        case 8:
        //widen scope

        //also the case here
        
        case 9:
        //focus scope
        if(loopPoint[i] - startPoint[i] >= 2) //check the math here...
        {
          startPoint[i] ++;
          loopPoint[i]--;
        }
        
        
        case 10:
        //repeat
        uint8_t nextPage = loopPoint[i] + 1;
        
        for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
        {
          
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            if(_NoteOn[j][i][k].degree != 15)
            {
              _NoteOn[j + nextPage][i][k].duration = _NoteOn[j][i][k].duration;
              _NoteOn[j + nextPage][i][k].degree = _NoteOn[j][i][k].degree;
              _NoteOn[j + nextPage][i][k].octave = _NoteOn[j][i][k].octave;           
            }
          }
        }
        loopPoint[i] = loopPoint[i] * 2 + 1;
        
        
        case 11:
        //retrograde

        uint8_t loc = (loopPoint[i] - startPoint[i]) * 2 + 1;
        
        for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
        {
          
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            if(_NoteOn[j][i][k].degree != 15)
            {
              _NoteOn[loc - j][i][k].duration = _NoteOn[j][i][k].duration;
              _NoteOn[loc - j][i][k].degree = _NoteOn[j][i][k].degree;
              _NoteOn[loc - j][i][k].octave = _NoteOn[j][i][k].octave;           
            }
          }
        }
        loopPoint[i] = loc;
        
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
  */

}

void Engine::destruction(uint8_t algorithm)
{
  /*
  for (uint8_t i = 0; i < TRACKS - 1; i++)
  {
    if(writeEnabled[i])
    {
      switch(algorithm)
      {
        case 1:
        //erase random note
    
        bool complete2 = false;
        uint8_t newStep;
        uint8_t newPoly;
            
        while(!complete2)
        {
          newStep = random(startPoint[i], loopPoint[i]);
          newPoly = random(POLYPHONY);
          
          if(_NoteOn[newStep][i][newPoly].degree != 15)
          {
            
            _NoteOn[newStep][i][newPoly].degree = 15;
            _NoteOn[newStep][i][newPoly].octave = 0;
            _NoteOn[newStep][i][newPoly].duration = 0;
            complete2 = true;
                         
           }
            
         }
        case 2:
        //erase lowest notes
        
        for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
        {
              
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            if(_NoteOn[j][i][k].degree != 15)
            {
              if(lowest > _NoteOn[j][i][k].degree + _NoteOn[j][i][k].octave * 12 + key[i])
               lowest = _NoteOn[j][i][k].degree + _NoteOn[j][i][k].octave * 12 + key[i];
            }
          }
        }
        for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
        {
              
          for(uint8_t k = 0; k < POLYPHONY; k++)     ///MAKE BUILDNOTE FUNCTION AGAIN
          {
            if(_NoteOn[j][i][k].degree != 15 && )
            {
              if(lowest > _NoteOn[j][i][k].degree + _NoteOn[j][i][k].octave * 12 + key[i])
               lowest = _NoteOn[j][i][k].degree + _NoteOn[j][i][k].octave * 12 + key[i];
            }
          }
        }
        case 3:
        //erase highest notes
        case 4:
        //double space
    
        //needs copy array slot
        
        for(uint8_t j = 255; j > 0; j--)
        {
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            if(_NoteOn[j][i][k].degree != 15)
            
          }
        }
        
        case 5:
        //tripple space
        case 6:
        //big bang
        
        
        case 7:
        //crunch down remove even steps
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
        //total silence (re-init track)
    
        for(uint8_t j = 0; j < STEPS; j++)
        {
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            _NoteOn[j][i][k].degree = 15;
          }
        }
    
        case 15:
        //re-init scope within loop
        
        for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
        {
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            _NoteOn[j][i][k].degree = 15;
          }
        }
        
        default:
        break;
      }
    }
  }   
  */
}

uint8_t Engine::printScales(uint8_t x, uint8_t y)
{
  return table[x][y];
}

 
