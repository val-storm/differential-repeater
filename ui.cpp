#include "ui.h"

ui::ui() {
  
}

void ui::_begin(Engine * link) {
  
  longTouch = 0;
  sequencer = link;
}

/**********************************
*       user interface logic
**********************************/

void ui::readTouch(uint16_t touch) {

  uint8_t track = 0;
  unsigned long _now;


  //scan first five bits for pressence of a track code

  lowCode = touch & 0x1F;
  highCode = touch & 0xFE0;
  track = getTrack(lowCode);
    
  
  //default mode controls
  
  //set track record (hold selected track and tap record)
  if(lowCode && touch & RECORD_CODE)
  {
    if(track == 16)
    {
      sequencer->initializeTrack();
    } else if (track == 17)
    {
      sequencer->undoLastNote();
    } else 
    {
      sequencer->selectTrackRecord(track);
      return;
    }
  }

  if(lowCode && touch & PLAY_CODE)
  {
    if(track == 16)
    {
      sequencer->muteAll();
      return;
    } else if (track == 17)
    {
      sequencer->_stop();
      return;
    } else 
    {
      sequencer->muteTrack(track);
      return;
    }
  }
  
  if(lowCode && touch & UP_CODE)
  {
    if(track == 16)
    {
      //loop point 16 up on track record
      return;
    }
    if(track == 17)
    {
      //start point up 16 on track record
      return;
    }
    sequencer->pageUp(track);
    return;
  }
  
  if(lowCode && touch & DOWN_CODE)
  {
    if(track == 16)
    {
      //loop point 16 down on track record
      return;
    }
    if(track == 17)
    {
      //start point down 16 on track record
      return;
    }
    sequencer->pageUp(track);
    return;
  }
  //set track scale mode (hold track and tap scale)
  if(lowCode && touch & SCALE_CODE)
  {
    
    sequencer->setTrackScale(track);
    return;  
    
  }
  
  if(lowCode && touch & ALGORITHM_CODE)
  { 
    if(track < 4 || (track > 7 && track < 12))
      sequencer->toggleWriteEnabled(track);
    else
      sequencer->construction(track);
    return;  
  }

  if(lowCode && touch & UTIL_CODE)
  { 
    sequencer->changeDirection(track);
    return;  
  }
  
  //octave up
  if(touch & UP_CODE && !_up.current)
  {
    sequencer->octaveUp();
    _up.current = true;
    return;
  }

  if(!(touch & UP_CODE)  && _up.current)
  {
    _up.current = false;
   
  }
  
  //octave down
  if(touch & DOWN_CODE && !_down.current)
  {
    sequencer->octaveDown();
    _down.current = true;
    return;
  }

  if(!(touch & DOWN_CODE) && _down.current)
  {
    _down.current = false;
  }

  //play
  if(touch & PLAY_CODE && !_play.current)
  {
    sequencer->pause();
    _play.current = true;
    return;
  }

  if(!(touch & PLAY_CODE) && _play.current)
  {
    _play.current = false;
  }
  
  //toggle record
  if(!lowCode && touch & RECORD_CODE)
  {
    sequencer->toggleRecord();
    //_record.current = true;
    return;
  }
  if(touch == RESTART_CODE)
  {
    CPU_RESTART
  }


  
  
}

uint8_t ui::getTrack(uint8_t track)
{
  for(uint8_t i = 0; i < 18; i++)
  {
    if(trackCode[i] == track)
      return i;
  }
  return 0;
}

uint16_t ui::getStuff()
{
  return reg12;
}



