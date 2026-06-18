
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
#include <SettingsESP.h>
#include <Preferences.h>
#include "ds18b20.h"
#include "led.h"
#include "freqcounter.h"
//#include <ESP8266WiFi.h>


SettingsESP sett("Teplica Poliv");
Preferences preferences;

TApplication *App;
#if defined(ESP8266) || defined(ESP32)
TWiFiControl *AppWiFi;
TMQTTControl *AppMQTT;
#endif
static const int MOTOR_COUNT = 4;
static const int BUTTON_COUNT = 5;
static const int MOTOR_TIMER_MAX_MINUTES = 1440;

TButton *Btn[BUTTON_COUNT];
TButton *PumpBtn;
TLed *Led[BUTTON_COUNT];
TTimer *Timer1, *TimerMQTT;
TSensor_DS18B20 *Temp1, *Temp2;
TMotorDriver *MotorDriver[MOTOR_COUNT];
unsigned long MotorTimerStartedAt[MOTOR_COUNT];
bool MotorTimerActive[MOTOR_COUNT];
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
    char OTAPassword[64];
    char MotorName[MOTOR_COUNT][WEB_LABEL_LEN];
    int MotorTimerMinutes[MOTOR_COUNT];
    char Temp1Name[WEB_LABEL_LEN];
    char Temp2Name[WEB_LABEL_LEN];
    char BarrelName[WEB_LABEL_LEN];
    char PumpName[WEB_LABEL_LEN];
};

Data data;
