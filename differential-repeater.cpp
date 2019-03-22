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
    polycounter[i].bufferStep = 0;
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
    division[i] = & sixteenth;
  }
 

  //low and high filters will come in later with creation destruction algorithms

  lowFilter = 0;

  highFilter = 127;
 
  octave = 0;

  scale = 0;

  ringBuffer[0].location = 4;
  ringBuffer[1].location = 5;
  ringBuffer[2].location = 6;
  ringBuffer[3].location = 7;

  ringBuffer[0].isOn = true;
  
  previousKeys = 0;
  
  isRunning = false;

  isRecording = false;

  nextTick = 0;

  //nextBeat = 0;

  trackRecord = 0;
  

  loopPoint[4] = 255;
  
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

    nextBeat[i] = _now + *division[i];
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

  beatDiv_8 = 60000L / tempo / 2;

  beatDiv_8_triplets = 60000L / tempo / 3;

  beatDiv_32 = 60000L / tempo / 8;

  _clock = 60000L / tempo / 24;

// for (uint8_t i = 0; i < TRACKS; i++)
//  {
//    division[i] = sixteenth;
//  }
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

  bufferOverwrite();
  
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

  
  if(ringBuffer[track].isOn)
  {
    
    uint16_t i = seqPosition[track];
    uint8_t seqLength = loopPoint[track] - startPoint[track];
    do {
      //Serial.println("hey");
      triggerNotes(ringBuffer[track].location, i);
      i += seqLength;
      
    } while(i <= 255); 
  }
    

}

void Engine::writeNoteOn(uint8_t degree)
{ 
  
  
  //sound the scaled note
  
  midicb(table[trackScale[trackRecord]][degree] + octave * 12 + key[trackRecord], 1, trackRecord + 1);

  //quantize ring buffer write location
  polycounter[degree].bufferStep = quantize(ringBuffer[trackRecord].location);
  polycounter[degree].startTime = millis();
  

  //don't write if not recording
  if(!isRecording)
    return;
    
  //lock in a step to write to
  uint8_t quantizedPosition = quantize(trackRecord);
  polycounter[degree].startStep = quantizedPosition;
  
  
}

void Engine::writeNoteOff(uint8_t degree)
{
  //off the scaled note
  
  midicb(table[trackScale[trackRecord]][degree] + octave * 12 + key[trackRecord], 0, trackRecord + 1);

  //get duration
  uint8_t duration = (millis() - polycounter[degree].startTime) / * division[trackRecord];

  if(duration < 1)
    duration = 1;

  //write to ring buffer
  uint8_t bufferStep = polycounter[degree].bufferStep;

  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
    if(_NoteOn[bufferStep][ringBuffer[trackRecord].location][i].degree == 15)
    {
      _NoteOn[bufferStep][ringBuffer[trackRecord].location][i].duration = duration;
      _NoteOn[bufferStep][ringBuffer[trackRecord].location][i].degree = degree;
      _NoteOn[bufferStep][ringBuffer[trackRecord].location][i].octave = octave;
      break;
    }
    
  }

  //still don't record if not recording
  if(!isRecording)
    return;
    
  uint8_t writeStep = polycounter[degree].startStep;
  
  
    
  //find an open 'space' then stop looking for one
  
  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
    if(_NoteOn[writeStep][trackRecord][i].degree == 15)
    {
      _NoteOn[writeStep][trackRecord][i].duration = duration;
      _NoteOn[writeStep][trackRecord][i].degree = degree;
      _NoteOn[writeStep][trackRecord][i].octave = octave;
      //Serial.println("Hi");
      last = &_NoteOn[writeStep][trackRecord][i];
      break;
    }
    
  }
}

void Engine::undoLastNote()
{
  last->duration = 0;
  last->degree = 15;
  last->octave = 0;
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

void Engine::bufferOverwrite()
{
  for(uint8_t i = 4; i < 8; i++)
  {
    //Serial.println("hey");
    for(uint8_t j = 0; j < POLYPHONY; j++)
    {
      _NoteOn[seqPosition[i]][i][j].degree = 15;
    }
  }
  return;
}

void Engine::triggerNotes(uint8_t track)
{
  if(trackMute[track])
    return;
    
  uint8_t midiNote;
  uint8_t degree;

  //iterate over 'spaces' for initiated data
  
  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
    degree = _NoteOn[seqPosition[track]][track][i].degree;

    //just incase keep looking
    if(degree == 15)
      continue;

    //space has note data, scale the data
    midiNote = table[trackScale[track]][degree] + _NoteOn[seqPosition[track]][track][i].octave * 12 + key[track];

    //better check the "virtual key board" to see if the note is already sounding on this track
    if(keysOn[track][midiNote])
      continue;
    
    //all good, so, sound the note! *MIDI doesn't have track zero*
    midicb(midiNote, 1, (track + 1));

    //update keyboard
    keysOn[track][midiNote] = true;

    //update polyphony management array
    
    NoteOutbox[track][polyIndex[track]].duration =  _NoteOn[seqPosition[track]][track][i].duration;
    NoteOutbox[track][polyIndex[track]].note =  midiNote;

    //shift the index to accept the next note
    polyIndex[track] = polyIndex[track] + 1;
   }
    
}

//OVERLOADED TRIGGERNOTES FUNCTION FOR BUFFERS

void Engine::triggerNotes(uint8_t track, uint16_t bufferStep)
{
  if(trackMute[track])
    return;
    
  uint8_t midiNote;
  uint8_t degree;

  //iterate over 'spaces' for initiated data
  
  for(uint8_t i = 0; i < POLYPHONY; i++)
  {
    degree = _NoteOn[bufferStep][track][i].degree;

    //just incase keep looking
    if(degree == 15)
      continue;

    //space has note data, scale the data
    midiNote = table[trackScale[track]][degree] + _NoteOn[bufferStep][track][i].octave * 12 + key[track];

    //better check the "virtual key board" to see if the note is already sounding on this track
    if(keysOn[track][midiNote])
      continue;
    
    //all good, so, sound the note! *MIDI doesn't have track zero*
    midicb(midiNote, 1, (track + 1));

    //update keyboard
    keysOn[track][midiNote] = true;

    //update polyphony management array
    
    NoteOutbox[track][polyIndex[track]].duration =  _NoteOn[bufferStep][track][i].duration;
    NoteOutbox[track][polyIndex[track]].note =  midiNote;
    Serial.println(NoteOutbox[track][polyIndex[track]].duration);
    //shift the index to accept the next note
    polyIndex[track] = polyIndex[track] + 1;
   }
    
}
uint8_t Engine::quantize(uint8_t track)
{
  //Serial.println("hey");
  unsigned long writeNow = millis();

  unsigned long midpoint = *division[track] / 2;

  if(writeNow <= (nextBeat[track] - midpoint))
    return seqPosition[track];

  if((seqPosition[track] + 1) >= loopPoint[track])
    return startPoint[track];

  return seqPosition[track] + 1;
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
  return * division[track];
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

void Engine::pageUp(uint8_t track)
{

  startPoint[track] = (startPoint[track] + 16) % STEPS;
  loopPoint[track] = (loopPoint[track] + 16) % STEPS;
  seqPosition[track] = startPoint[track];
  
}

void Engine::pageDown(uint8_t track)
{

  startPoint[track] = (startPoint[track] - 16) % STEPS;
  loopPoint[track] = (loopPoint[track] - 16) % STEPS;
  seqPosition[track] = startPoint[track];
  
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

void Engine::muteTrack()
{
  trackMute[trackRecord] = trackMute[trackRecord] ? false : true;
}

void Engine::muteTrack(uint8_t track)
{
  trackMute[track] = trackMute[track] ? false : true;
}

void Engine::muteAll()
{
  for(uint8_t i = 0; i < TRACKS; i++)
  {
    trackMute[i] = false;
  }
}

void Engine::initializeTrack()
{
  _stop();

  uint8_t i = 0;
 
  do {
    for(uint8_t j = 0; j < POLYPHONY; j++)
    {
      _NoteOn[i][trackRecord][j].degree = 15;
    }
  } while(i++ != 255);
  
}

void Engine::setDivision()
{
  //later
}

void Engine::setLoopPoint(uint8_t point)
{
  loopPoint[trackRecord] = point;
  
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
  
  for (uint8_t i = 0; i < TRACKS - 1; i++)
  {
    if(writeEnabled[i])
    {
      switch(algorithm)
      {
        case 0:
         toggleWriteEnabled(0);
        break;

        case 1:
          toggleWriteEnabled(1);
        
        case 2:
          
          break;
          
        case 3:
         toggleWriteEnabled(3);
         break;
        
        case 4:
        //add random note

          bool complete_random_add;
          complete_random_add = false;
          uint8_t newStep_random_add;
          uint8_t newPoly_random_add;
          
          while(!complete_random_add)
          {
            newStep_random_add = random(startPoint[i], loopPoint[i]);
            newPoly_random_add = random(POLYPHONY);
            
            if(_NoteOn[newStep_random_add][i][newPoly_random_add].degree == 15)
            {
              
              uint8_t newNote_random_add = random(12);
              _NoteOn[newStep_random_add][i][newPoly_random_add].degree = newNote_random_add;
              _NoteOn[newStep_random_add][i][newPoly_random_add].octave = octave;
              _NoteOn[newStep_random_add][i][newPoly_random_add].duration = (millis() % 3) + 1;
  
                           
             }
              
           }
           break;
        
        case 5:
        
        //retrograde fugue
          
          uint8_t loc_retro_fugue;
          loc_retro_fugue = ((loopPoint[i] - startPoint[i]) * 2 + 1);
          
          for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
          {
            for(uint8_t k = 0; k < POLYPHONY; k++)
            {
              if(_NoteOn[j][i][k].degree != 15)
              {
                if((_NoteOn[j][i][k].degree + 4) > 11) //change 4 to a DEFINE and create a fugue harmony variable that can be changed
                {
                  _NoteOn[loc_retro_fugue - j][i][k].duration = _NoteOn[j][i][k].duration;
                  _NoteOn[loc_retro_fugue - j][i][k].degree = (_NoteOn[j][i][k].degree + 4) % 8;
                  _NoteOn[loc_retro_fugue - j][i][k].octave = _NoteOn[j][i][k].octave + 1;
                  
                } else {
                  _NoteOn[loc_retro_fugue - j][i][k].duration = _NoteOn[j][i][k].duration;
                  _NoteOn[loc_retro_fugue - j][i][k].degree = _NoteOn[j][i][k].degree + 4;
                  _NoteOn[loc_retro_fugue - j][i][k].octave = _NoteOn[j][i][k].octave;
                  
                }
              }
            }
          }
          //startPoint[i] = loopPoint[i] + 1;
          loopPoint[i] = loc_retro_fugue;
          seqPosition[i] = startPoint[i];
          Serial.println("Hey");
          break;
        
        case 6:
        
        //the "maybe copy"
        
          uint8_t maybe;
          uint8_t nextPage_maybe;
          nextPage_maybe = loopPoint[i] + 1;
  
          for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
          {
            for(uint8_t k = 0; k < POLYPHONY; k++)
            {
              maybe = random() % 5;
              
              if(_NoteOn[j][i][k].degree != 15 && maybe)
              {
                _NoteOn[j + nextPage_maybe][i][k].duration = _NoteOn[j][i][k].duration;
                _NoteOn[j + nextPage_maybe][i][k].degree = _NoteOn[j][i][k].degree;
                _NoteOn[j + nextPage_maybe][i][k].octave = _NoteOn[j][i][k].octave;           
              }
            }
          }
          loopPoint[i] = (loopPoint[i] * 2) + 1;
          //startPoint[i] = nextPage_maybe;
          seqPosition[i] = startPoint[i];
          break;
        
        case 7:
         //basic fugue
          uint8_t loc_fugue;
          loc_fugue = ((loopPoint[i] - startPoint[i]) * 2 + 1);
          
          for(uint8_t j = startPoint[i]; j < loopPoint[i]; j++)
          {
            for(uint8_t k = 0; k < POLYPHONY; k++)
            {
              if(_NoteOn[j][i][k].degree != 15)
              {
                if((_NoteOn[j][i][k].degree + 4) > 11)
                {
                  _NoteOn[(j + loc_fugue)][i][k].duration = _NoteOn[j][i][k].duration;
                  _NoteOn[(j + loc_fugue)][i][k].degree = (_NoteOn[j][i][k].degree + 4) % 8;
                  _NoteOn[(j + loc_fugue)][i][k].octave = _NoteOn[j][i][k].octave + 1;
                  
                } else {
                  _NoteOn[(j + loc_fugue)][i][k].duration = _NoteOn[j][i][k].duration;
                  _NoteOn[(j + loc_fugue)][i][k].degree = _NoteOn[j][i][k].degree + 4;
                  _NoteOn[(j + loc_fugue)][i][k].octave = _NoteOn[j][i][k].octave;
                  
                }
              }
            }
          }
          loopPoint[i] = loc_fugue;
          //startPoint[i] = startPoint[i] + 16;
          seqPosition[i] = startPoint[i];
          break;
          
        case 8:
        //widen scope

        //also the case here
          break;
        case 9:
        //focus scope
          toggleWriteEnabled(9);
          break;
        
        case 10:
        
          break;
        
        
        case 11:

          break;
        
        case 12:
        //repeat
          uint8_t nextPage_repeat;
          nextPage_repeat = loopPoint[i] + 1;
          
          for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
          {
            
            for(uint8_t k = 0; k < POLYPHONY; k++)
            {
              if(_NoteOn[j][i][k].degree != 15)
              {
                _NoteOn[j + nextPage_repeat][i][k].duration = _NoteOn[j][i][k].duration;
                _NoteOn[j + nextPage_repeat][i][k].degree = _NoteOn[j][i][k].degree;
                _NoteOn[j + nextPage_repeat][i][k].octave = _NoteOn[j][i][k].octave;           
              }
            }
          }
          loopPoint[i] = (loopPoint[i] * 2) + 1;
          //startPoint[i] = nextPage_repeat;
          seqPosition[i] = startPoint[i];
          break;
        
        case 13:
        //retrograde

          uint8_t loc_retro;
          loc_retro = (loopPoint[i] - startPoint[i]) * 2 + 1;
          
          for(uint8_t j = startPoint[i]; j <= loopPoint[i]; j++)
          {
            
            for(uint8_t k = 0; k < POLYPHONY; k++)
            {
              if(_NoteOn[j][i][k].degree != 15)
              {
                _NoteOn[loc_retro - j][i][k].duration = _NoteOn[j][i][k].duration;
                _NoteOn[loc_retro - j][i][k].degree = _NoteOn[j][i][k].degree;
                _NoteOn[loc_retro - j][i][k].octave = _NoteOn[j][i][k].octave;           
              }
            }
          }
          loopPoint[i] = loc_retro;
          startPoint[i] = startPoint[i] + 16;
          seqPosition[i] = startPoint[i];
          break;
          
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
           break;
      }
    }
  }
  

}

void Engine::destruction(uint8_t algorithm)
{
  
  for (uint8_t i = 0; i < TRACKS - 1; i++)
  {
    if(writeEnabled[i])
    {
      switch(algorithm)
      {
        case 1:
        //erase random note
    
        bool complete_random_erase;
        complete_random_erase = false;
        uint8_t newStep_random_erase;
        uint8_t newPoly_random_erase;
            
        while(!complete_random_erase)
        {
          newStep_random_erase = random(startPoint[i], loopPoint[i]);
          newPoly_random_erase = random(POLYPHONY);
          
          if(_NoteOn[newStep_random_erase][i][newPoly_random_erase].degree != 15)
          {
            
            _NoteOn[newStep_random_erase][i][newPoly_random_erase].degree = 15;
            _NoteOn[newStep_random_erase][i][newPoly_random_erase].octave = 0;
            _NoteOn[newStep_random_erase][i][newPoly_random_erase].duration = 0;
            complete_random_erase = true;
                         
           }
            
         }
        case 2:
        //erase lowest notes
        /*
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
        */
        case 3:
        //erase highest notes
        case 4:
        //double space
    
        //needs copy array slot
        /*
        for(uint8_t j = 255; j > 0; j--)
        {
          for(uint8_t k = 0; k < POLYPHONY; k++)
          {
            if(_NoteOn[j][i][k].degree != 15)
            
          }
        }
        */
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
  
}

uint8_t Engine::printScales(uint8_t x, uint8_t y)
{
  return table[x][y];
}

 
