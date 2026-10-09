#pragma once
#include <stdint.h>
#include <cmath>
struct Saver{
 bool on=false,latched=false;uint8_t confirmations=0;uint32_t idleSince=0;
 void tick(uint32_t now,bool valid,float current,bool loud){
  if(!valid||!std::isfinite(current)||current<0||current>=500){on=false;latched=true;confirmations=0;return;}
  if(latched)return;
  confirmations=loud?(confirmations<2?confirmations+1:2):0;
  if(confirmations>=2&&!on){on=true;idleSince=now;}
  if(on){if(loud||current>=80)idleSince=now;else if(uint32_t(now-idleSince)>=30000)on=false;}
 }
 bool reset(bool safe){if(!safe)return false;latched=false;on=false;confirmations=0;return true;}
 void stop(){on=false;latched=true;}
};
