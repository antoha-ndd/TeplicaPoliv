
#include "var.h"
#include "events.h"


void Init()
{

    App = new TApplication();
    App->Run();


    Btn[0] = new TButton(18, true);
    Btn[0]->OnPress = Button1_OnClick;
    Btn[0]->Register(App);

    Btn[1] = new TButton(19, true);
    Btn[1]->OnPress = Button2_OnClick;
    Btn[1]->Register(App);

    Btn[2] = new TButton(21, true);
    Btn[2]->OnPress = Button3_OnClick;
    Btn[2]->Register(App);

    Btn[3] = new TButton(35, true);
    Btn[3]->OnPress = Button4_OnClick;
    Btn[3]->Register(App);

    Btn[4] = new TButton(34, true);
    Btn[4]->OnPress = Button5_OnClick;
    Btn[4]->Register(App);

    Limiter = new TButton(13, false);
    Limiter->Register(App);
    

    Led[0] = new TLed(2);
    

    Led[1] = new TLed(4);
    Led[1]->Off();

    Led[2] = new TLed(16);
    Led[2]->Off();

    Led[3] = new TLed(17);
    Led[3]->Off();

    Led[4] = new TLed(5);
    Led[4]->Off();

    Temp1 = new TSensor_DS18B20(15);
    Temp2 = new TSensor_DS18B20(0);

    Pump = new TOutputDevice(23);
    Pump->Register(App);
    Pump->OnChageState = Pump_OnChageState;
    Pump->Off();

    MotorDriver[0] = new TMotorDriver(12, 27);
    MotorDriver[1] = new TMotorDriver(14, 33);
    MotorDriver[2] = new TMotorDriver(32, 25);
    MotorDriver[3] = new TMotorDriver(22, 26);

    MotorDriver[0]->OnChageState = Motor1_OnChageState;
    MotorDriver[1]->OnChageState = Motor2_OnChageState;
    MotorDriver[2]->OnChageState = Motor3_OnChageState;
    MotorDriver[3]->OnChageState = Motor4_OnChageState;

    for (size_t i = 0; i < 4; i++)
    {
        MotorDriver[i]->Register(App);
        MotorDriver[i]->AutoClose = false;
        MotorDriver[i]->AutoOpen = false;
        MotorDriver[i]->Timeout = 30000;
        MotorDriver[i]->InitClose();
    }

    Fc = new TFreqCounter(1);
    Fc->Register(App);
    
    Fc->OnChangeFreq = Fc_OnChangeFreq;

    ui.start();
    ui.attachBuild(build);
    ui.attach(action);

    LoadSettings();


    Timer1 = new TTimer();
    Timer1->Register(App);
    Timer1->Start(100);
    Timer1->OnTimeout = Timer1_Timeout;
}
