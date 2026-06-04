
#pragma once
#define GH_NO_PAIRS

#include <espwifi.h>
#include "Objects.h"
#include "app/WiFiControl.h"
#include "app/MQTTControl.h"
#include "ObjectTimer.h"
#include "button.h"
#include "varbasetypes.h"
#include "MotorDriver.h"
#include <GyverPortal.h>
#include <Preferences.h>
#include "ds18b20.h"
#include "led.h"
#include "freqcounter.h"
//#include <ESP8266WiFi.h>


GyverPortal ui;
Preferences preferences;

TApplication *App;
#if defined(ESP8266) || defined(ESP32)
TWiFiControl *AppWiFi;
TMQTTControl *AppMQTT;
#endif
TButton *Btn[5];
TButton *PumpBtn;
TLed *Led[5];
TTimer *Timer1, *TimerMQTT;
TSensor_DS18B20 *Temp1, *Temp2;
TMotorDriver *MotorDriver[4];
TOutputDevice *Pump;
TButton *Limiter;




#define WEB_LABEL_LEN 64

struct Data
{
    int Port;
    char MQTTServer[100];
    char MQTTTopic[100];
    float MinWaterTemp;
    char WiFiSSID[64];
    char WiFiPassword[64];
    char MotorName[4][WEB_LABEL_LEN];
    char Temp1Name[WEB_LABEL_LEN];
    char Temp2Name[WEB_LABEL_LEN];
    char BarrelName[WEB_LABEL_LEN];
    char PumpName[WEB_LABEL_LEN];
};

Data data;
