#pragma once
#include "stack.h"

#if defined(ESP8266) || defined(ESP32)
#include "espwifi.h"
#include <WiFiClient.h>
#include <PubSubClient.h>
#endif

class TApplication;

typedef unsigned long  TTimeStamp;

typedef unsigned int ObjectId;
typedef void (*TApplicationPrintCallback)(TApplication *App, String Buffer);

struct TSystemMessage{
	String Type;
	String Name;
	String Param;
};

void DefaultApplicationPrintCallBack(TApplication *App, String Buffer)
{
	Serial.print(Buffer);
}

void DefaultApplicationPrintLnCallBack(TApplication *App, String Buffer)
{
	Serial.println(Buffer);
}

#if defined(ESP8266) || defined(ESP32)
void Pairing()
{
	WiFiManager wifiManager;
	wifiManager.resetSettings();
	wifiManager.autoConnect("AutoConnectAP");
}
#endif

void EmptryEvent() {};

unsigned long int EmptyTimer() { return millis(); };
unsigned long int (*GetTimerValue)(){EmptyTimer};

ObjectId LastObjectId{0};

ObjectId GetGlobalIDValue()
{

	LastObjectId++;
	return LastObjectId;
}

class TObject;
class TApplication;
class TControl;
class TActiveControl;

class TObject
{
private:
	TObject *Parent;
	ObjectId ID;

public:
	TStack<TObject *> *Children;

	TObject(TObject *_Parent);
	~TObject();
	void AddChild(TObject *Child);
	void RemoveChild(TObject *Child);

	ObjectId GetID()
	{
		return ID;
	};

	TObject GetParent()
	{
		return Parent;
	}
};

TObject::TObject(TObject *_Parent)
{
	this->ID = GetGlobalIDValue();
	this->Parent = _Parent;

	Children = new TStack<TObject *>();

	if (Parent != NULL)
	{
		Parent->AddChild(this);
	}
}

TObject::~TObject()
{
	if (Parent != NULL)
	{
		Parent->RemoveChild(this);
	}
}

void TObject::AddChild(TObject *Child)
{

	Children->Add(Child);
}

void TObject::RemoveChild(TObject *Child)
{
	Children->Delete(Child);
}

class TApplication : public TObject
{
private:
	TActiveControl *CurrentActiveControl{NULL};
	bool isRun{false};
	TApplicationPrintCallback PrintCb{DefaultApplicationPrintCallBack};
	TApplicationPrintCallback PrintLnCb{DefaultApplicationPrintLnCallBack};
	
	#if defined(ESP8266) || defined(ESP32)
	WiFiClient MQTTClient;
	PubSubClient mqtt;
	String MQTT_Server;
	int MQTT_Port;
	String MQTT_Topic;
	bool MQTT_Intialized{false};
	static TApplication* MQTT_APP_instance;


	static void MQTT_callback(char *topic, byte *payload, unsigned int length){
		
		String Topic = String(topic);
		String Payload ="";
			
		for (int i = 0; i < length; i++)
			Payload += (char)payload[i];

		MQTT_APP_instance->MQTT_ProcessMessage(Topic,Payload); 
    }

    void MQTT_ProcessMessage(String topic, String payload);

	#endif
	

public:
	uint64_t Tick{0};
	TStack<TControl *> *Controls;
	bool DoStop{false};

	#if defined(ESP8266) || defined(ESP32)

		int MQTT_Timeout{5000};
		
		void InitMQTT(String Server, int Port, String Topic){
			MQTT_Server = Server;
			MQTT_Port = Port;
			MQTT_Topic = Topic;

			mqtt.setClient(MQTTClient);
			mqtt.setServer(MQTT_Server.c_str(), MQTT_Port);
			mqtt.setCallback(MQTT_callback);
			MQTT_Intialized = true;
			MQTT_APP_instance = this;

		}

		void SubScribe(String topic){

			mqtt.subscribe(topic.c_str());

		}

		void Publish(String Topic, String Payload){

			mqtt.publish(Topic.c_str(), Payload.c_str());

		}

		void reconnect()
		{
			// Loop until we're reconnected
			while (!mqtt.connected())
			{
				String clientId = "ESP8266Client-";
				clientId += String(random(0xffff), HEX);
		
				if (mqtt.connect(clientId.c_str()))
				{
					mqtt.subscribe( (MQTT_Topic+"/#").c_str());
				}
				else
					delay(5000);
			}
		}

	#endif

	void Print(String Buffer)
	{

		PrintCb(this, Buffer);
	}

	void PrintLn(String Buffer)
	{

		PrintLnCb(this, Buffer);
	}

	void SetPrintLnProcedure(TApplicationPrintCallback Cb)
	{
		PrintLnCb = Cb;
	}

	void SetPrintProcedure(TApplicationPrintCallback Cb)
	{
		PrintCb = Cb;
	}

	bool AppIsRun()
	{
		return isRun;
	}

	TApplication() : TObject(NULL)
	{
		Controls = new TStack<TControl *>();
		

#if defined(ESP8266) || defined(ESP32) 

		WiFi.begin();
		WiFiManager wifiManager;
		wifiManager.setDebugOutput(false);
		wifiManager.autoConnect("AutoConnectAP");

		ArduinoOTA.setPort(8266);
		ArduinoOTA.setHostname("ESP_Board");
		ArduinoOTA.setPassword("8764956");
		ArduinoOTA.begin();

	
#endif

	}

	~TApplication() {}

	void Stop();

	void NeedAction();

	void AddControl(TControl *Control);

	long int ControlsCount()
	{

		return Controls->GetCount();
	}

	void DeleteControl(TControl *Control);

	void Idle();
	uint64_t GetTick()
	{
		return Tick;
	}

	void Run()
	{
		Tick = 0;
		DoStop = false;
		isRun = true;

	}

	void (*OnIdle)(){EmptryEvent};
};

class TControl : public TObject
{
protected:
	TApplication *Application{NULL};


public:
	TControl(TObject *_Parent) : TObject(_Parent) {};
	TControl() : TObject(NULL) {};
	virtual void SystemMessage(TSystemMessage msg){};
	TApplication *GetApplication()
	{
		return Application;
	}

	void Register(TApplication *App)
	{
		Application = App;
		Application->AddControl(this);
	}

	~TControl();

	virtual void Idle()
	{
		OnIdle();
	}
	void (*OnIdle)(){EmptryEvent};
};

TControl::~TControl()
{
	Application->DeleteControl(this);
}


void TApplication::DeleteControl(TControl *Control)
{
	Controls->Delete(Control);
}

void TApplication::Stop()
{
	isRun = false;
	DoStop = true;

	TControl *CurrentControl{NULL};
	CurrentControl = Controls->GetNext(CurrentControl);
}

void TApplication::Idle()
{
	TTimeStamp CurrentTime = millis();
#if defined(ESP8266) || defined(ESP32)
	static unsigned long WiFiConnectionTimeout = 0;

	ArduinoOTA.handle();

	if (WiFi.status() != WL_CONNECTED)
	{
		
		if ((CurrentTime - WiFiConnectionTimeout) >= 10000)
		{
			WiFiConnectionTimeout = CurrentTime;
			Serial.println("Try reconnect WiFi");
			WiFi.reconnect();
		}
	}

	if(MQTT_Intialized){

		static TTimeStamp MQTT_Last_Tick=0;

		if( (CurrentTime - MQTT_Last_Tick) > MQTT_Timeout){

			MQTT_Last_Tick = CurrentTime;
			reconnect();

		}

	}

#endif

	if (DoStop)
		return;

	TControl *CurrentControl{NULL};
	CurrentControl = Controls->GetNext(CurrentControl);

	while (CurrentControl != NULL)
	{
		if (DoStop)
			return;
		CurrentControl->Idle();
		CurrentControl = Controls->GetNext(CurrentControl);
	}

	OnIdle();
	Tick++;
}

void TApplication::AddControl(TControl *Control)
{
	Controls->Add(Control);
}



void TApplication::MQTT_ProcessMessage(String topic, String payload)
{
	TControl *CurrentControl{NULL};
	CurrentControl = Controls->GetNext(CurrentControl);

	while (CurrentControl != NULL)
	{
		TSystemMessage msg;
		msg.Type = "MQTT";
		msg.Name = topic;
		msg.Param = payload;

		CurrentControl->SystemMessage(msg);
		CurrentControl = Controls->GetNext(CurrentControl);
	}

}