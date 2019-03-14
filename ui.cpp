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

  
  //reset long touch counter if register is empty
  if(!touch) {
    longTouch = 0;
  }

  //start counting if any controls are pressed
  if(touch) {
    _now = millis(); //start counter
    longTouch = _now + LONG_TIME;
  }

  //scan first five bits for pressence of a track code

  lowCode = touch & 0x1F;
  track = getTrack(lowCode);
    

  //default mode controls

  //set track record (hold selected track and tap record)
  if(track && touch & RECORD_CODE)
  {
    sequencer->selectTrackRecord(track - 1);
    return;
  }

  //set track scale mode (hold track and tap scale)
  if(track && touch & SCALE_CODE)
  {
    
    sequencer->setTrackScale(track - 1);
    return;  
    
  }
  
  if(track && touch & ALGORITHM_CODE)
  { 
    switch (track)
    {
     case 17:
      sequencer->construction(1);
     case 18:
      sequencer->construction(5); 
     default: 
      sequencer->toggleWriteEnabled(track - 1);
    }
    return;  
    
  }

  if(track && touch & UTIL_CODE)
  { 
    sequencer->setDirection(track - 1, 0x2);
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
  if(touch & RECORD_CODE && !_record.current)
  {
    sequencer->toggleRecord();
    _record.current = true;
    return;
  }

  if(!(touch & RECORD_CODE) && _record.current)
  {
    _record.current = false;
    
  }

  //toggle record
  if(touch & STOP_CODE && _play.current)
  {
    sequencer->_stop();
    _play.current = false;
    return;
  }

  
  
}

uint8_t ui::getTrack(uint8_t track)
{
  for(uint8_t i = 0; i < 18; i++)
  {
    if(trackCode[i] == track)
      return i + 1;
  }
  return 0;
}

uint16_t ui::getStuff()
{
  return reg12;
}



