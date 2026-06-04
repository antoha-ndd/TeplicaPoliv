
#include <Arduino.h>

#include "AppSettings.h"



void setup()
{
  
  Init();
}

void loop()
{
  static bool LoopStarted = false;

  if (!LoopStarted) {
    LoopStarted = true;
    Serial.println("[System] Loop started");
  }

  ui.tick();
  App->Idle();
 /* 
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();*/
}
