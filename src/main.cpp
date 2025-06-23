
#include <Arduino.h>

String pl="";

#include "AppSettings.h"



void setup()
{
  
  Init();
}

void loop()
{

  App->Idle();
  ui.tick();
  
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();
}
