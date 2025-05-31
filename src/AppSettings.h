
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

GyverPortal ui;
Preferences preferences;

TApplication *App;
TButton *Btn[5];
TTimer *Timer1;

TSensor_DS18B20 *Temp1, *Temp2;

TMotorDriver *MotorDriver[3];
// ConfigWebServer configServer(80);

struct Data
{
    int Port;
    char MQTTServer[100];
    char MQTTTopic[100];
    float o[3], c[3];
    bool ac[3], ao[3];
};
Data data;


void BtnOpen1_Click(TButton *Button)
{

    MotorDriver[0]->Open();
};

void BtnClose1_Click(TButton *Button)
{

    MotorDriver[0]->Close();
};

void BtnOpen2_Click(TButton *Button)
{

    MotorDriver[1]->Open();
};

void BtnClose2_Click(TButton *Button)
{
    MotorDriver[1]->Close();
};

void BtnOpen3_Click(TButton *Button)
{

    MotorDriver[2]->Open();
};

void BtnClose3_Click(TButton *Button)
{

    MotorDriver[2]->Close();
};


void LoadSettings()
{
/*
    preferences.begin("config", false);

    data.c[0] = preferences.getFloat("c1", 0);
    data.c[1] = preferences.getFloat("c2", 0);
    data.c[2] = preferences.getFloat("c3", 0);
    data.o[0] = preferences.getFloat("o1", 0);
    data.o[1] = preferences.getFloat("o2", 0);
    data.o[2] = preferences.getFloat("o3", 0);

    data.ac[0] = preferences.getBool("ac1", false);
    data.ac[1] = preferences.getBool("ac2", false);
    data.ac[2] = preferences.getBool("ac3", false);

    data.ao[0] = preferences.getBool("ao1", false);
    data.ao[1] = preferences.getBool("ao2", false);
    data.ao[2] = preferences.getBool("ao3", false);

    data.Port = preferences.getInt("Port", 1883);
    strcpy(data.MQTTServer, preferences.getString("Server", "").c_str());
    strcpy(data.MQTTTopic, preferences.getString("Topic", "").c_str());

    MotorDriver[0]->AutoClose = data.ac[0];
    MotorDriver[1]->AutoClose = data.ac[1];
    MotorDriver[2]->AutoClose = data.ac[2];

    MotorDriver[0]->AutoOpen = data.ao[0];
    MotorDriver[1]->AutoOpen = data.ao[1];
    MotorDriver[2]->AutoOpen = data.ao[2];

    preferences.end();*/
}

void build()
{

    LoadSettings();

    GP.BUILD_BEGIN(GP_DARK, 500);

    GP.LABEL("Температура 1 : " + String( Temp1->Temperature(true)));
    GP.LABEL("Температура 2 : " + String( Temp2->Temperature(true)));
    GP.BREAK();

/*
    GP.BUTTON("SaveBtn", "Сохранить");

    GP_MAKE_BLOCK_TAB("Окно 1",

                      GP_MAKE_BOX(
                          GP.LABEL("Открыто");
                          GP.SWITCH("", true, GP_GREEN, true););

                      GP_MAKE_BOX(
                          GP_MAKE_BOX(
                              GP.LABEL("Закрытие");
                              GP.NUMBER_F("c1", "", data.c[0]););

                          GP_MAKE_BOX(
                              GP.LABEL("Открытие");
                              GP.NUMBER_F("o1", "", data.o[0])););

                      GP_MAKE_BOX(

                          GP_MAKE_BOX(
                              GP.LABEL("Автооткрытие");
                              GP.SWITCH("ao1", data.ao[0]););

                          GP_MAKE_BOX(
                              GP.LABEL("Автозакрытие");
                              GP.SWITCH("ac1", data.ac[0]));

                      );

                      GP_MAKE_BOX(
                          GP.BUTTON("Open1", "Открыть");
                          GP.BUTTON("Close1", "Закрыть"););

    );

    GP_MAKE_BLOCK_TAB("Окно 2",

                      GP_MAKE_BOX(
                          GP_MAKE_BOX(
                              GP.LABEL("Закрытие");
                              GP.NUMBER_F("c2", "", data.c[1]););

                          GP_MAKE_BOX(
                              GP.LABEL("Открытие");
                              GP.NUMBER_F("o2", "", data.o[1])););

                      GP_MAKE_BOX(
                          GP_MAKE_BOX(
                              GP.LABEL("Автооткрытие");
                              GP.SWITCH("ao2", data.ao[1]););

                          GP_MAKE_BOX(
                              GP.LABEL("Автозакрытие");
                              GP.SWITCH("ac2", data.ac[1])););

                      GP_MAKE_BOX(
                          GP.BUTTON("Open2", "Открыть");
                          GP.BUTTON("Close2", "Закрыть");););

    GP_MAKE_BLOCK_TAB("Дверь",
                      GP_MAKE_BOX(
                          GP_MAKE_BOX(
                              GP.LABEL("Закрытие");
                              GP.NUMBER_F("c3", "", data.c[2]););

                          GP_MAKE_BOX(
                              GP.LABEL("Открытие");
                              GP.NUMBER_F("o3", "", data.o[2])););
                      GP_MAKE_BOX(
                          GP_MAKE_BOX(
                              GP.LABEL("Автооткрытие");
                              GP.SWITCH("ao3", data.ao[2]););

                          GP_MAKE_BOX(
                              GP.LABEL("Автозакрытие");
                              GP.SWITCH("ac3", data.ac[2])););
                      GP_MAKE_BOX(
                          GP.BUTTON("Open3", "Открыть");
                          GP.BUTTON("Close3", "Закрыть"););

    );

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

    GP.BUILD_END();*/
}

void action()
{
    /*
    if (ui.click())
    {
        // по клику переписать пришедшие данные в переменные
        ui.clickFloat("c1", data.c[0]);
        ui.clickFloat("c2", data.c[1]);
        ui.clickFloat("c3", data.c[2]);
        ui.clickFloat("o1", data.o[0]);
        ui.clickFloat("o2", data.o[1]);
        ui.clickFloat("o3", data.o[2]);

        ui.clickBool("ao1", data.ao[0]);
        ui.clickBool("ao2", data.ao[1]);
        ui.clickBool("ao3", data.ao[2]);

        ui.clickBool("ac1", data.ac[0]);
        ui.clickBool("ac2", data.ac[1]);
        ui.clickBool("ac3", data.ac[2]);

        ui.clickStr("MQTTServer", data.MQTTServer);
        ui.clickStr("MQTTTopic", data.MQTTTopic);
        ui.clickInt("MQTTPort", data.Port);

        if (ui.click("SaveBtn"))
        {

            preferences.begin("config", false);
            preferences.putFloat("c1", data.c[0]);
            preferences.putFloat("c2", data.c[1]);
            preferences.putFloat("c3", data.c[2]);
            preferences.putFloat("o1", data.o[0]);
            preferences.putFloat("o2", data.o[1]);
            preferences.putFloat("o3", data.o[2]);

            preferences.putBool("ac1", data.ac[0]);
            preferences.putBool("ac2", data.ac[1]);
            preferences.putBool("ac3", data.ac[2]);

            preferences.putBool("ao1", data.ao[0]);
            preferences.putBool("ao2", data.ao[1]);
            preferences.putBool("ao3", data.ao[2]);

            preferences.putInt("Port", data.Port);
            preferences.putString("Server", data.MQTTServer);
            preferences.putString("Topic", data.MQTTTopic);

            preferences.end();

            MotorDriver[0]->AutoClose = data.ac[0];
            MotorDriver[1]->AutoClose = data.ac[1];
            MotorDriver[2]->AutoClose = data.ac[2];

            MotorDriver[0]->AutoOpen = data.ao[0];
            MotorDriver[1]->AutoOpen = data.ao[1];
            MotorDriver[2]->AutoOpen = data.ao[2];
        }

        if (ui.click("RebootBtn"))
            ESP.restart();

        if (ui.click("Open1"))
            MotorDriver[0]->Open();

        if (ui.click("Open2"))
            MotorDriver[1]->Open();

        if (ui.click("Open3"))
            MotorDriver[2]->Open();

        if (ui.click("Close1"))
            MotorDriver[0]->Close();

        if (ui.click("Close2"))
            MotorDriver[1]->Close();

        if (ui.click("Close3"))
            MotorDriver[2]->Close();
    }*/
}


void Timer1_Timeout(TTimer *Timer){

    static bool State{false};
    digitalWrite(2 , State);
    State = !State;

};

void Init()
{

    App = new TApplication();
    App->Run();

    pinMode(2,OUTPUT);

    Btn[0] = new TButton(NULL, 18, true);
    Btn[0]->OnPress = BtnOpen1_Click;
    Btn[0]->Register(App);

    Btn[1] = new TButton(NULL, 19, true);
    Btn[1]->OnPress = BtnOpen2_Click;
    Btn[1]->Register(App);

    Btn[2] = new TButton(NULL, 21, true);
    Btn[2]->OnPress = BtnOpen3_Click;
    Btn[2]->Register(App);

    Btn[3] = new TButton(NULL, 22, true);
    Btn[3]->OnPress = BtnClose1_Click;
    Btn[3]->Register(App);

    Btn[4] = new TButton(NULL, 23, true);
    Btn[4]->OnPress = BtnClose2_Click;
    Btn[4]->Register(App);


    Temp1 = new TSensor_DS18B20(15);
    Temp2 = new TSensor_DS18B20(0);

    Timer1 = new TTimer();
    Timer1->Register(App);
    Timer1->Start(1000);
    Timer1->OnTimeout = Timer1_Timeout;
     

    /*
   
    MotorDriver[0] = new TMotorDriver(14, 27);
    MotorDriver[1] = new TMotorDriver(12, 13);
    MotorDriver[2] = new TMotorDriver(2, 26);

    MotorDriver[0]->Register(App);
    MotorDriver[1]->Register(App);
    MotorDriver[2]->Register(App);

    MotorDriver[0]->InitClose();
    MotorDriver[1]->InitClose();
    MotorDriver[2]->InitClose();

    MotorDriver[0]->AutoClose = true;
    MotorDriver[0]->AutoOpen = true;

    MotorDriver[1]->AutoClose = true;
    MotorDriver[1]->AutoOpen = true;

    MotorDriver[2]->AutoClose = true;
    MotorDriver[2]->AutoOpen = true;*/

    ui.start();
    ui.attachBuild(build);
    ui.attach(action);

    LoadSettings();
}

