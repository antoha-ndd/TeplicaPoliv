#include "var.h"



void MQTT_Motor1(String payload){

    if(payload=="1") MotorDriver[0]->Open();
    else MotorDriver[0]->Close();
};

void MQTT_Motor2(String payload){

    if(payload=="1") MotorDriver[1]->Open();
    else MotorDriver[1]->Close();


};

void MQTT_Motor3(String payload){

    if(payload=="1") MotorDriver[2]->Open();
    else MotorDriver[2]->Close();

};

void MQTT_Motor4(String payload){

    if(payload=="1") MotorDriver[3]->Open();
    else MotorDriver[3]->Close();

};

void MQTT_Pump(String payload){

    if(payload=="1") Pump->On();
    else Pump->Off();

};


void Button1_OnClick(TButton *Button)
{
    int n = 0;

    if (MotorDriver[n]->IsOpen())
        MotorDriver[n]->Close();
    else
        MotorDriver[n]->Open();
};

void Button2_OnClick(TButton *Button)
{

    int n = 1;

    if (MotorDriver[n]->IsOpen())
        MotorDriver[n]->Close();
    else
        MotorDriver[n]->Open();
};

void Button3_OnClick(TButton *Button)
{
    int n = 2;

    if (MotorDriver[n]->IsOpen())
        MotorDriver[n]->Close();
    else
        MotorDriver[n]->Open();
};

void Button4_OnClick(TButton *Button)
{
    int n = 3;

    if (MotorDriver[n]->IsOpen())
        MotorDriver[n]->Close();
    else
        MotorDriver[n]->Open();
};

void Button5_OnClick(TButton *Button)
{

    Pump->Toggle();
};

void Pump_OnChageState(TSimpleDevice *Device, bool State)
{
    //if(mqtt->connected()) mqtt->publish(String(String(data.MQTTTopic)+"/Pump").c_str(), String(Pump->GetState()) );
    Led[4]->SetState(Device->GetState());
};



void Motor1_OnChageState(TMotorDriver *Device)
{
    //if(mqtt->connected()) mqtt->publish(String(String(data.MQTTTopic)+"/Motor1").c_str(), String(MotorDriver[0]->IsOpen()) );

    Led[0]->SetState(Device->IsOpen());
};

void Motor2_OnChageState(TMotorDriver *Device)
{
    //if(mqtt->connected()) mqtt->publish(String(String(data.MQTTTopic)+"/Motor2").c_str(), String(MotorDriver[1]->IsOpen()) );
    Led[1]->SetState(Device->IsOpen());
};

void Motor3_OnChageState(TMotorDriver *Device)
{
    //if(mqtt->connected()) mqtt->publish(String(String(data.MQTTTopic)+"/Motor3").c_str(), String(MotorDriver[2]->IsOpen()) );
    Led[2]->SetState(Device->IsOpen());
};

void Motor4_OnChageState(TMotorDriver *Device)
{
    //if(mqtt->connected()) mqtt->publish(String(String(data.MQTTTopic)+"/Motor4").c_str(), String(MotorDriver[3]->IsOpen()) );
    Led[3]->SetState(Device->IsOpen());
};

void LoadSettings()
{

    preferences.begin("config", false);

    data.Port = preferences.getInt("Port", 1883);
    data.MinWaterTemp = preferences.getFloat("MinWaterTemp", 0);
    strcpy(data.MQTTServer, preferences.getString("Server", "").c_str());
    strcpy(data.MQTTTopic, preferences.getString("Topic", "").c_str());

    preferences.end();
}

void build()
{

    LoadSettings();

    GP.BUILD_BEGIN(GP_DARK, 500);

    GP.LABEL("Температура 1 : " + String(Temp1->Temperature(true)));
    GP.LABEL("Температура 2 : " + String(Temp2->Temperature(true)));
    GP.LABEL("Бочка полна : " + String(Limiter->GetState()));
    GP.BREAK();

    GP.BUTTON("SaveBtn", "Сохранить");

    GP_MAKE_BLOCK_TAB("MQTT",
                      GP_MAKE_BOX(
                          GP.LABEL("Cервер");
                          GP.TEXT("MQTTServer", "", data.MQTTServer););

                      GP_MAKE_BOX(
                          GP.LABEL("Порт");
                          GP.NUMBER("MQTTPort", "", data.Port));

                      GP_MAKE_BOX(
                          GP.LABEL("Топик");
                          GP.TEXT("MQTTTopic", "", data.MQTTTopic);););

    GP.BUTTON("RebootBtn", "Перезагрузить");

    GP.BUILD_END();
}

void action()
{

    if (ui.click())
    {

        ui.clickStr("MQTTServer", data.MQTTServer);
        ui.clickStr("MQTTTopic", data.MQTTTopic);
        ui.clickInt("MQTTPort", data.Port);

        if (ui.click("SaveBtn"))
        {

            preferences.begin("config", false);

            preferences.putInt("Port", data.Port);
            preferences.putString("Server", data.MQTTServer);
            preferences.putString("Topic", data.MQTTTopic);

            preferences.end();
            
  //          mqtt->UpdateSetting(data.MQTTServer , data.Port, data.MQTTTopic);
        }

        if (ui.click("RebootBtn"))
            ESP.restart();
    }
}

void Timer1_Timeout(TTimer *Timer)
{

    if (!Limiter->GetState())
        MotorDriver[3]->Close();


};


/*
void TimerMQTT_Timeout(TTimer *Timer)
{
    if(mqtt->connected()){

        mqtt->publish(String(String(data.MQTTTopic)+"/Motor1").c_str(), String(MotorDriver[0]->IsOpen()) );
        mqtt->publish(String(String(data.MQTTTopic)+"/Motor2").c_str(), String(MotorDriver[1]->IsOpen()) );
        mqtt->publish(String(String(data.MQTTTopic)+"/Motor3").c_str(), String(MotorDriver[2]->IsOpen()) );
        mqtt->publish(String(String(data.MQTTTopic)+"/Motor4").c_str(), String(MotorDriver[3]->IsOpen()) );

        mqtt->publish(String(String(data.MQTTTopic)+"/Pump").c_str(), String(Pump->GetState()) );
        mqtt->publish(String(String(data.MQTTTopic)+"/Limiter").c_str(), String(Limiter->GetState()) );

        mqtt->publish(String(String(data.MQTTTopic)+"/GroundTemp").c_str(), String(Temp1->Temperature(true)) );
        mqtt->publish(String(String(data.MQTTTopic)+"/WaterTemp").c_str(), String(Temp2->Temperature(true)) ); 

    }
    
};

*/

void setupWiFi()
{

    WiFiManager wm; // Создаем объект WiFiManager

    wm.setConnectTimeout(30);       // Таймаут подключения 30 секунд
    wm.setConfigPortalTimeout(180); // Автоматическое закрытие портала через 180 секунд

    // Запускаем режим конфигурации
    if (!wm.startConfigPortal("AutoconnectAP"))
    {                  // Задаем имя AP
        ESP.restart(); // Перезагрузка в случае неудачи
        delay(1000);
    }
}