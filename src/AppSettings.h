
#include "var.h"
#include "events.h"
#include "hardware_pins.h"

#if defined(ESP8266) || defined(ESP32)
const unsigned long SERIAL_MONITOR_SPEED = 57600;
#endif

unsigned long int ArduinoTimerValue()
{
	return millis();
}

TTimerValueCallback GetTimerValue = ArduinoTimerValue;

void Init()
{
	LoadSettings();

#if defined(ESP8266) || defined(ESP32)
	Serial.begin(SERIAL_MONITOR_SPEED);
	delay(50);
	Serial.println();
	Serial.println("[System] Boot");
#endif

	App = new TApplication();
	App->Run();

#if defined(ESP8266) || defined(ESP32)
	AppWiFi = new TWiFiControl();
	AppWiFi->Register(App);

	AppMQTT = new TMQTTControl();
	AppMQTT->Register(App);
#endif

	Btn[0] = new TButton(PIN_BTN_0, true);
	Btn[0]->OnPress = Button1_OnClick;
	Btn[0]->Register(App);

	Btn[1] = new TButton(PIN_BTN_1, true);
	Btn[1]->OnPress = Button2_OnClick;
	Btn[1]->Register(App);

	Btn[2] = new TButton(PIN_BTN_2, true);
	Btn[2]->OnPress = Button3_OnClick;
	Btn[2]->Register(App);

	Btn[3] = new TButton(PIN_BTN_3, true);
	Btn[3]->OnPress = Button4_OnClick;
	Btn[3]->Register(App);

	Btn[4] = new TButton(PIN_BTN_4, true);
	Btn[4]->OnPress = Button5_OnClick;
	Btn[4]->Register(App);

	Limiter = new TButton(PIN_LIMITER, false);
	Limiter->Register(App);

	Led[0] = new TLed(PIN_LED_0);
	Led[0]->Off();
	Led[1] = new TLed(PIN_LED_1);
	Led[1]->Off();
	Led[2] = new TLed(PIN_LED_2);
	Led[2]->Off();
	Led[3] = new TLed(PIN_LED_3);
	Led[3]->Off();
	Led[4] = new TLed(PIN_LED_4);
	Led[4]->Off();

	Temp1 = new TSensor_DS18B20(PIN_TEMP_1);
	Temp2 = new TSensor_DS18B20(PIN_TEMP_2);

	Pump = new TOutputDevice(PIN_PUMP);
	Pump->InvertState = true;
	Pump->Register(App);
	Pump->OnChageState = Pump_OnChageState;
	Pump->Off();

	MotorDriver[0] = new TMotorDriver(PIN_MOTOR0_OPEN, PIN_MOTOR0_CLOSE);
	MotorDriver[1] = new TMotorDriver(PIN_MOTOR1_OPEN, PIN_MOTOR1_CLOSE);
	MotorDriver[2] = new TMotorDriver(PIN_MOTOR2_OPEN, PIN_MOTOR2_CLOSE);
	MotorDriver[3] = new TMotorDriver(PIN_MOTOR3_OPEN, PIN_MOTOR3_CLOSE);

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

	PumpBtn = new TButton(PIN_PUMP_BTN);
	PumpBtn->OnPress = Button5_OnClick;
	PumpBtn->Register(App);

	ui.attachBuild(build);
	ui.attach(action);
	ui.start();

	Serial.print("[Web] UI http://");
#if defined(ESP8266) || defined(ESP32)
	Serial.println(AppWiFi->SoftAPIP());
#else
	Serial.println();
#endif

	Timer1 = new TTimer();
	Timer1->Register(App);
	Timer1->Start(100);
	Timer1->OnTimeout = Timer1_Timeout;

	TimerMQTT = new TTimer();
	TimerMQTT->Register(App);
	TimerMQTT->Start(5000);
	TimerMQTT->OnTimeout = TimerMQTT_Timeout;

#if defined(ESP8266) || defined(ESP32)
	if (strlen(data.MQTTServer) > 0)
	{
		String ttopic = String(data.MQTTTopic);
		if (ttopic.length() == 0)
			ttopic = "teplica";
		AppMQTT->InitMQTT(String(data.MQTTServer), data.Port, ttopic);
	}
#endif

	Serial.println("[System] Init complete");
}
