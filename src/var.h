
#pragma once
#define GH_NO_PAIRS

#include <espwifi.h>
#include "Objects.h"
#include "ObjectTimer.h"
#include "button.h"
#include "varbasetypes.h"
#include "MotorDriver.h"
#include <GyverPortal.h>
#include <Preferences.h>
#include "ds18b20.h"
#include "tmqttclient.h"
#include "led.h"
#include "freqcounter.h"

GyverPortal ui;
Preferences preferences;

TFreqCounter *Fc;
TApplication *App;
TButton *Btn[5];
TLed *Led[5];
TTimer *Timer1;
TSensor_DS18B20 *Temp1, *Temp2;
TMotorDriver *MotorDriver[4];
TOutputDevice *Pump;
TButton *Limiter;

struct Data
{
    int Port;
    char MQTTServer[100];
    char MQTTTopic[100];
    float MinWaterTemp;
};

Data data;
