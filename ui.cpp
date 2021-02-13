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
      return;
    } else if (track == 17)
    {
      sequencer->undoLastNote();
      return;
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
  
   
  //set track scale mode (hold track and tap scale)
  if(lowCode && touch & SCALE_CODE)
  {
    
    sequencer->setTrackScale(track);
    return;  
    
  }
  
  if(lowCode && touch & ALGORITHM_CODE)
  { 
    if(track < 4 || (track > 7 && track < 12))
      //sequencer->toggleWriteEnabled(track);
      return;
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
  if(touch & UP_CODE && !lowCode)
  {
    sequencer->octaveUp();
    return;
  }

  if(touch & UP_CODE  && lowCode)
  {
   if(track == 16)
    {
      sequencer->incStartPoint();
      return;
    } else if (track == 17)
    {
      sequencer->incLoopPoint();
      return;
    } else 
    {
      sequencer->pageUp(track);
      return;
    }
   
  }
  
  //octave down
  if(touch & DOWN_CODE && !lowCode)
  {
    sequencer->octaveDown();
    return;
  }

  if(touch & DOWN_CODE && lowCode)
  {
    if(track == 16)
    {
      sequencer->decStartPoint();
      return;
    } else if (track == 17)
    {
      sequencer->decLoopPoint();
      return;
    } else 
    {
      sequencer->pageDown(track);
      return;
    }
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
  for(uint8_t i = 0; i < 17; i++)
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



