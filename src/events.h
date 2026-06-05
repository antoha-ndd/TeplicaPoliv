#include "var.h"
#include <cstring>

enum WebTabs : uint8_t
{
    WebTabPanel,
    WebTabWifi,
    WebTabMqtt
};

enum WebIds : size_t
{
    WebTabsId = "WebTabs"_h,
    WebLblT1 = "WebLblT1"_h,
    WebLblT2 = "WebLblT2"_h,
    WebLblBarrel = "WebLblBarrel"_h,
    WebLblM1 = "WebLblM1"_h,
    WebLblM2 = "WebLblM2"_h,
    WebLblM3 = "WebLblM3"_h,
    WebLblM4 = "WebLblM4"_h,
    WebLblPump = "WebLblPump"_h,
    WebTemp1 = "WebTemp1"_h,
    WebTemp2 = "WebTemp2"_h,
    WebBarrelLed = "WebBarrelLed"_h,
    WebMotor1Led = "WebMotor1Led"_h,
    WebMotor2Led = "WebMotor2Led"_h,
    WebMotor3Led = "WebMotor3Led"_h,
    WebMotor4Led = "WebMotor4Led"_h,
    WebPumpLed = "WebPumpLed"_h,
    WebMqttLed = "WebMqttLed"_h,
    WebMotor1Open = "WebMotor1Open"_h,
    WebMotor1Close = "WebMotor1Close"_h,
    WebMotor2Open = "WebMotor2Open"_h,
    WebMotor2Close = "WebMotor2Close"_h,
    WebMotor3Open = "WebMotor3Open"_h,
    WebMotor3Close = "WebMotor3Close"_h,
    WebMotor4Open = "WebMotor4Open"_h,
    WebMotor4Close = "WebMotor4Close"_h,
    WebPumpOn = "WebPumpOn"_h,
    WebPumpOff = "WebPumpOff"_h,
    WiFiSSID = "WiFiSSID"_h,
    WiFiPass = "WiFiPass"_h,
    OTAPass = "OTAPass"_h,
    MQTTServer = "MQTTServer"_h,
    MQTTPort = "MQTTPort"_h,
    MQTTTopic = "MQTTTopic"_h,
    RebootBtn = "RebootBtn"_h,
    SaveBtn = "SaveBtn"_h,
    WebDashRefresh = "WebDashRefresh"_h
};

static uint8_t webCurrentTab = WebTabPanel;

/** Бочка полна: активный уровень — LOW (GetState() == false). */
inline bool BarrelIsFull()
{
    return Limiter != nullptr && !Limiter->GetState();
}

static void webCopyStr(char *dst, const char *src, size_t len)
{
    if (!dst || len == 0)
        return;
    if (!src)
    {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, len - 1);
    dst[len - 1] = '\0';
}

static void webTrimLabel(char *s)
{
    if (!s)
        return;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t'))
        s[--n] = '\0';
    size_t i = 0;
    while (s[i] == ' ' || s[i] == '\t')
        i++;
    if (i > 0)
        memmove(s, s + i, strlen(s + i) + 1);
}

static void webLabelDefaults()
{
    const char *motors[] = {"М1", "М2", "М3", "М4"};
    for (int i = 0; i < 4; i++)
        webCopyStr(data.MotorName[i], motors[i], WEB_LABEL_LEN);
    webCopyStr(data.Temp1Name, "T1", WEB_LABEL_LEN);
    webCopyStr(data.Temp2Name, "T2", WEB_LABEL_LEN);
    webCopyStr(data.BarrelName, "Бочка полна", WEB_LABEL_LEN);
    webCopyStr(data.PumpName, "Насос", WEB_LABEL_LEN);
}

static void webLoadLabel(char *dst, const char *key, const char *fallback)
{
    String s = preferences.getString(key, "");
    if (s.length() > 0)
        webCopyStr(dst, s.c_str(), WEB_LABEL_LEN);
    else
        webCopyStr(dst, fallback, WEB_LABEL_LEN);
}

static void webSaveLabelNames()
{
    preferences.begin("config", false);
    preferences.putString("lbl_m1", data.MotorName[0]);
    preferences.putString("lbl_m2", data.MotorName[1]);
    preferences.putString("lbl_m3", data.MotorName[2]);
    preferences.putString("lbl_m4", data.MotorName[3]);
    preferences.putString("lbl_t1", data.Temp1Name);
    preferences.putString("lbl_t2", data.Temp2Name);
    preferences.putString("lbl_barrel", data.BarrelName);
    preferences.putString("lbl_pump", data.PumpName);
    preferences.end();
}

static String formatTempC(TSensor_DS18B20 *sensor)
{
    if (!sensor)
        return String("—");
    float t = sensor->Temperature(false);
    if (t == DEVICE_DISCONNECTED_C)
        return String("—");
    return String(t, 1) + " °C";
}

static void webNormalizeLabel(char *value, const char *fallback)
{
    webTrimLabel(value);
    if (strlen(value) == 0)
        webCopyStr(value, fallback, WEB_LABEL_LEN);
}

static void webSaveConnectionSettings()
{
    preferences.begin("config", false);

    preferences.putInt("Port", data.Port);
    preferences.putString("Server", data.MQTTServer);
    preferences.putString("Topic", data.MQTTTopic);
    preferences.putString("wifi_ssid", data.WiFiSSID);
    preferences.putString("wifi_pass", data.WiFiPassword);
    preferences.putString("ota_pass", data.OTAPassword);

    preferences.end();
}

static void webApplyConnectionSettings()
{
#if defined(ESP8266) || defined(ESP32)
    if (AppWiFi)
        AppWiFi->ApplySettingsFromNvs();
#endif

    if (!AppMQTT)
        return;

    if (strlen(data.MQTTServer) > 0)
    {
        String ttopic = String(data.MQTTTopic);
        if (ttopic.length() == 0)
            ttopic = "teplica";
        AppMQTT->InitMQTT(String(data.MQTTServer), data.Port, ttopic);
    }
    else
    {
        AppMQTT->MqttShutdown();
    }
}

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
	if (AppMQTT && AppMQTT->IsMQTTConnected())
		AppMQTT->PublishUnderTopic("state/pump", State ? "1" : "0");
	Led[4]->SetState(Device->GetState());
};



void Motor1_OnChageState(TMotorDriver *Device)
{
	if (AppMQTT && AppMQTT->IsMQTTConnected())
		AppMQTT->PublishUnderTopic("state/motor1", Device->IsOpen() ? "1" : "0");
	Led[0]->SetState(Device->IsOpen());
};

void Motor2_OnChageState(TMotorDriver *Device)
{
	if (AppMQTT && AppMQTT->IsMQTTConnected())
		AppMQTT->PublishUnderTopic("state/motor2", Device->IsOpen() ? "1" : "0");
	Led[1]->SetState(Device->IsOpen());
};

void Motor3_OnChageState(TMotorDriver *Device)
{
	if (AppMQTT && AppMQTT->IsMQTTConnected())
		AppMQTT->PublishUnderTopic("state/motor3", Device->IsOpen() ? "1" : "0");
	Led[2]->SetState(Device->IsOpen());
};

void Motor4_OnChageState(TMotorDriver *Device)
{
	if (AppMQTT && AppMQTT->IsMQTTConnected())
		AppMQTT->PublishUnderTopic("state/motor4", Device->IsOpen() ? "1" : "0");
	Led[3]->SetState(Device->IsOpen());
};

static bool webLabelInput(sets::Builder &b, size_t id, const char *label, char *value, const char *fallback)
{
    if (!b.Input(id, label, AnyPtr(value, WEB_LABEL_LEN)))
        return false;

    webNormalizeLabel(value, fallback);
    webSaveLabelNames();
    return true;
}

void LoadSettings()
{
    webLabelDefaults();
    preferences.begin("config", false);

    data.Port = preferences.getInt("Port", 1883);
    data.MinWaterTemp = preferences.getFloat("MinWaterTemp", 0);

    {
        String s = preferences.getString("Server", "");
        strncpy(data.MQTTServer, s.c_str(), sizeof(data.MQTTServer) - 1);
        data.MQTTServer[sizeof(data.MQTTServer) - 1] = '\0';
    }
    {
        String s = preferences.getString("Topic", "");
        strncpy(data.MQTTTopic, s.c_str(), sizeof(data.MQTTTopic) - 1);
        data.MQTTTopic[sizeof(data.MQTTTopic) - 1] = '\0';
    }
    {
        String s = preferences.getString("wifi_ssid", "");
        strncpy(data.WiFiSSID, s.c_str(), sizeof(data.WiFiSSID) - 1);
        data.WiFiSSID[sizeof(data.WiFiSSID) - 1] = '\0';
    }
    {
        String s = preferences.getString("wifi_pass", "");
        strncpy(data.WiFiPassword, s.c_str(), sizeof(data.WiFiPassword) - 1);
        data.WiFiPassword[sizeof(data.WiFiPassword) - 1] = '\0';
    }
    {
        String s = preferences.getString("ota_pass", "");
        if (s.length() == 0)
            s = String(OTA_DEFAULT_PASSWORD);
        strncpy(data.OTAPassword, s.c_str(), sizeof(data.OTAPassword) - 1);
        data.OTAPassword[sizeof(data.OTAPassword) - 1] = '\0';
    }

    webLoadLabel(data.Temp1Name, "lbl_t1", "T1");
    webLoadLabel(data.Temp2Name, "lbl_t2", "T2");
    webLoadLabel(data.BarrelName, "lbl_barrel", "Бочка полна");
    webLoadLabel(data.PumpName, "lbl_pump", "Насос");
    webLoadLabel(data.MotorName[0], "lbl_m1", "М1");
    webLoadLabel(data.MotorName[1], "lbl_m2", "М2");
    webLoadLabel(data.MotorName[2], "lbl_m3", "М3");
    webLoadLabel(data.MotorName[3], "lbl_m4", "М4");

    preferences.end();
}

static void webBuildSensors(sets::Builder &b)
{
    sets::Group group(b, "Датчики");

    {
        sets::Row row(b);
        webLabelInput(b, WebLblT1, "", data.Temp1Name, "T1");
        b.Label(WebTemp1, "", formatTempC(Temp1));
    }
    {
        sets::Row row(b);
        webLabelInput(b, WebLblT2, "", data.Temp2Name, "T2");
        b.Label(WebTemp2, "", formatTempC(Temp2));
    }
    {
       sets::Row row(b);
        webLabelInput(b, WebLblBarrel, "", data.BarrelName, "Бочка полна");
        b.LED(WebBarrelLed, "", BarrelIsFull());
    }
}

static void webBuildMotorRow(sets::Builder &b, int index, size_t labelId, size_t openId, size_t closeId, size_t ledId, const char *fallback)
{
    sets::Row row(b);
    webLabelInput(b, labelId, "", data.MotorName[index], fallback);
    if (b.Button(openId, "Откр", sets::Colors::Green) && MotorDriver[index])
        MotorDriver[index]->Open();
    if (b.Button(closeId, "Закр", sets::Colors::Green) && MotorDriver[index])
        MotorDriver[index]->Close();
    b.LED(ledId, "", MotorDriver[index] && MotorDriver[index]->IsOpen());
}

static void webBuildMotors(sets::Builder &b)
{
    sets::Group group(b, "Моторы");
    webBuildMotorRow(b, 0, WebLblM1, WebMotor1Open, WebMotor1Close, WebMotor1Led, "М1");
    webBuildMotorRow(b, 1, WebLblM2, WebMotor2Open, WebMotor2Close, WebMotor2Led, "М2");
    webBuildMotorRow(b, 2, WebLblM3, WebMotor3Open, WebMotor3Close, WebMotor3Led, "М3");
    webBuildMotorRow(b, 3, WebLblM4, WebMotor4Open, WebMotor4Close, WebMotor4Led, "М4");
}

static void webBuildPump(sets::Builder &b)
{
    sets::Group group(b, "Насос");
    sets::Row row(b);
    webLabelInput(b, WebLblPump, "", data.PumpName, "Насос");
    if (b.Button(WebPumpOn, "Вкл", sets::Colors::Green) && Pump)
        Pump->On();
    if (b.Button(WebPumpOff, "Выкл", sets::Colors::Green) && Pump)
        Pump->Off();
    b.LED(WebPumpLed, "", Pump && Pump->GetState());
}

static void webBuildPanel(sets::Builder &b)
{
    webBuildSensors(b);
    webBuildMotors(b);
    webBuildPump(b);
}

static void webBuildWifi(sets::Builder &b)
{
    sets::Group group(b, "Wi-Fi");
    b.Paragraph("После смены SSID/пароля нажмите «Сохранить».");
    b.Input(WiFiSSID, "SSID", AnyPtr(data.WiFiSSID, sizeof(data.WiFiSSID)));
    b.Pass(WiFiPass, "Пароль", AnyPtr(data.WiFiPassword, sizeof(data.WiFiPassword)));
    b.Pass(OTAPass, "OTA пароль", AnyPtr(data.OTAPassword, sizeof(data.OTAPassword)));
}

static void webBuildMqtt(sets::Builder &b)
{
    sets::Group group(b, "MQTT");
    b.Input(MQTTServer, "Сервер", AnyPtr(data.MQTTServer, sizeof(data.MQTTServer)));
    b.Number(MQTTPort, "Порт", &data.Port, 1, 65535);
    b.Input(MQTTTopic, "Топик", AnyPtr(data.MQTTTopic, sizeof(data.MQTTTopic)));
#if defined(ESP8266) || defined(ESP32)
    b.LED(WebMqttLed, "MQTT", AppMQTT && AppMQTT->IsMQTTConnected());
#endif
}

static void webBuildActions(sets::Builder &b)
{
    sets::Buttons buttons(b);

    if (b.Button(RebootBtn, "Перезагрузить", sets::Colors::Orange))
        ESP.restart();

    if (b.Button(SaveBtn, "Сохранить", sets::Colors::Green))
    {
        webSaveConnectionSettings();
        webApplyConnectionSettings();
    }

    if (b.Button(WebDashRefresh, "Обновить", sets::Colors::Gray))
        b.reload();
}

void build(sets::Builder &b)
{
    if (b.Tabs(WebTabsId, "Панель;Wi-Fi;MQTT", &webCurrentTab))
    {
        b.reload();
        return;
    }

    switch (webCurrentTab)
    {
    case WebTabWifi:
        webBuildWifi(b);
        break;
    case WebTabMqtt:
        webBuildMqtt(b);
        break;
    case WebTabPanel:
    default:
        webBuildPanel(b);
        break;
    }

    webBuildActions(b);
}

void update(sets::Updater &upd)
{
    upd.update(WebLblT1, data.Temp1Name);
    upd.update(WebLblT2, data.Temp2Name);
    upd.update(WebLblBarrel, data.BarrelName);
    upd.update(WebLblM1, data.MotorName[0]);
    upd.update(WebLblM2, data.MotorName[1]);
    upd.update(WebLblM3, data.MotorName[2]);
    upd.update(WebLblM4, data.MotorName[3]);
    upd.update(WebLblPump, data.PumpName);

    upd.update(WebTemp1, formatTempC(Temp1));
    upd.update(WebTemp2, formatTempC(Temp2));
    upd.update(WebBarrelLed, BarrelIsFull() ? 1 : 0);
    upd.update(WebMotor1Led, MotorDriver[0] && MotorDriver[0]->IsOpen() ? 1 : 0);
    upd.update(WebMotor2Led, MotorDriver[1] && MotorDriver[1]->IsOpen() ? 1 : 0);
    upd.update(WebMotor3Led, MotorDriver[2] && MotorDriver[2]->IsOpen() ? 1 : 0);
    upd.update(WebMotor4Led, MotorDriver[3] && MotorDriver[3]->IsOpen() ? 1 : 0);
    upd.update(WebPumpLed, Pump && Pump->GetState() ? 1 : 0);
#if defined(ESP8266) || defined(ESP32)
    upd.update(WebMqttLed, AppMQTT && AppMQTT->IsMQTTConnected() ? 1 : 0);
#endif
}

void Timer1_Timeout(TTimer *Timer)
{

    if (!Limiter->GetState())
        MotorDriver[3]->Close();


};



void TimerMQTT_Timeout(TTimer *Timer)
{
    if (Temp1)
        Temp1->Temperature(true);
    if (Temp2)
        Temp2->Temperature(true);
    yield();

#if defined(ESP8266) || defined(ESP32)
    if (AppMQTT && AppMQTT->IsMQTTConnected())
    {
        if (Temp1)
            AppMQTT->PublishUnderTopic("state/temp1", String(Temp1->Temperature(false), 2));
        if (Temp2)
            AppMQTT->PublishUnderTopic("state/temp2", String(Temp2->Temperature(false), 2));
        if (Limiter)
            AppMQTT->PublishUnderTopic("state/limiter", BarrelIsFull() ? "1" : "0");
        AppMQTT->PublishUnderTopic("state/minwater", String(data.MinWaterTemp, 1));
    }
#endif
}

#if defined(ESP8266) || defined(ESP32)

void AppMQTTProcessMessage(String topic, String payload)
{
    if (!App || strlen(data.MQTTServer) == 0)
        return;

    String base = String(data.MQTTTopic);
    if (base.length() == 0)
        base = "teplica";

    payload.trim();
    if (topic != base && !topic.startsWith(base + "/"))
        return;

    String sub = topic.substring(base.length());
    if (sub.startsWith("/"))
        sub = sub.substring(1);
    sub.toLowerCase();

    if (sub == "cmd/motor1" || sub == "motor1")
        MQTT_Motor1(payload);
    else if (sub == "cmd/motor2" || sub == "motor2")
        MQTT_Motor2(payload);
    else if (sub == "cmd/motor3" || sub == "motor3")
        MQTT_Motor3(payload);
    else if (sub == "cmd/motor4" || sub == "motor4")
        MQTT_Motor4(payload);
    else if (sub == "cmd/pump" || sub == "pump")
        MQTT_Pump(payload);
}

void MqttPublishFullState()
{
    if (!AppMQTT || !AppMQTT->IsMQTTConnected())
        return;
    if (Temp1)
        AppMQTT->PublishUnderTopic("state/temp1", String(Temp1->Temperature(false), 2));
    if (Temp2)
        AppMQTT->PublishUnderTopic("state/temp2", String(Temp2->Temperature(false), 2));
    if (Limiter)
        AppMQTT->PublishUnderTopic("state/limiter", BarrelIsFull() ? "1" : "0");
    for (int i = 0; i < 4; i++)
    {
        if (MotorDriver[i])
            AppMQTT->PublishUnderTopic(String("state/motor") + String(i + 1), MotorDriver[i]->IsOpen() ? "1" : "0");
    }
    if (Pump)
        AppMQTT->PublishUnderTopic("state/pump", Pump->GetState() ? "1" : "0");
    AppMQTT->PublishUnderTopic("state/minwater", String(data.MinWaterTemp, 1));
}

#endif
