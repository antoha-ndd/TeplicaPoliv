#include "var.h"
#include <cstring>

/** Бочка полна: активный уровень — LOW (GetState() == false). */
inline bool BarrelIsFull()
{
    return Limiter != nullptr && !Limiter->GetState();
}

static void webActuatorTableBegin()
{
    GP.TABLE_BEGIN("82%,18%", GP_ALS(GP_LEFT, GP_CENTER), "100%");
}

static void webActuatorTableHeader(const char *btnOn, const char *btnOff)
{
    GP.TR();
    GP.TD(GP_LEFT);
    GP.LABEL("");
    GP.TD(GP_CENTER);
    GP.LABEL(btnOn);
    GP.TD(GP_CENTER);
    GP.LABEL(btnOff);
    GP.TD(GP_CENTER);
    GP.LABEL("");
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

static String webJsEscape(const String &s)
{
    String r = s;
    r.replace("\\", "\\\\");
    r.replace("'", "\\'");
    r.replace("\r", "");
    r.replace("\n", "\\n");
    return r;
}

static void webNameLabel(const char *labelId, const char *text)
{
    GP.SEND(String(F("<span class='webNameLbl' id='")) + labelId + F("'>") + text + F("</span>"));
}

static void webRenameBtn(const char *btnId, const char *promptId, const char *labelId, const char *promptTitle, const char *text)
{
    GP.SEND(String(F("<button type='button' class='miniButton gp-rename-btn' id='")) + btnId +
            F("' onclick=\"var r=prompt('") + webJsEscape(promptTitle) + F("','") + webJsEscape(text) +
            F("');if(r!==null&&r!=='')GP_renameSend('") + promptId + F("',r,'") + labelId +
            F("');\">...</button>"));
}

static void webNameCell(const char *btnId, const char *promptId, const char *promptTitle, const char *labelId, const char *text)
{
    GP.SEND(F("<div class='webNameRow'>"));
    webRenameBtn(btnId, promptId, labelId, promptTitle, text);
    webNameLabel(labelId, text);
    GP.SEND(F("</div>"));
}

static void webAppLayoutStyle()
{
    GP.SEND(F("<style>"
              "#grid{display:grid;grid-template-columns:minmax(0,2fr) minmax(420px,1fr);align-items:start;gap:16px;width:min(96vw,1600px)!important;max-width:1600px!important}"
              "#grid>.block{min-width:0;width:auto!important;max-width:none!important;margin:10px 4px!important}"
              "#grid>.block:nth-of-type(1){grid-column:1;grid-row:1 / span 2}"
              "#grid>.block:nth-of-type(2){grid-column:2;grid-row:1}"
              "#grid>.block:nth-of-type(3){grid-column:2;grid-row:2}"
              "@media screen and (max-width:768px){"
              "#grid{display:block!important}"
              "#grid>.block{width:unset!important;margin:20px 5px!important}"
              "}"
              "</style>"));
}

static void webPanelStyle()
{
    GP.SEND(F("<script>"
              "function GP_renameSend(pid,val,lbl){"
              "var x=new XMLHttpRequest();x.open('POST','/GP_click?'+pid+'='+encodeURIComponent(val),true);"
              "x.timeout=_tout;x.onreadystatechange=function(){"
              "if(x.readyState===4&&x.status===200&&lbl)GP_update(lbl);};x.send();}"
              "</script>"
              "<style>"
              ".webPanel table td{padding:12px 6px;vertical-align:middle}"
              ".webPanel>label{display:block;margin:18px 0 12px}"
              ".webPanel hr{margin:22px 0}"
              ".webNameRow{display:flex;align-items:center;gap:6px}"
              ".webNameLbl{display:inline-block;line-height:1.4}"
              ".gp-rename-btn{background:#4a5568!important;min-width:28px!important;padding:2px 5px!important;margin:0!important}"
              ".webCtrlBtns{display:flex;justify-content:flex-end;align-items:center;gap:10px;width:100%}"
              ".webCtrlBtns .miniButton{margin:0!important;min-width:44px;padding:4px 6px}"
              ".webValWrap,.webStatusWrap{display:flex;justify-content:center;align-items:center;width:100%}"
              ".webValLbl{display:inline-block;text-align:center}"
              "</style>"));
}

static void webValueCell(const char *valueId, const String &text)
{
    GP.TD(GP_CENTER);
    GP.SEND(F("<div class='webValWrap'>"));
    GP.SEND(String(F("<span class='webValLbl' id='")) + valueId + F("'>") + text + F("</span>"));
    GP.SEND(F("</div>"));
}

static void webStatusCell(const char *ledId, bool state)
{
    GP.TD(GP_CENTER);
    GP.SEND(F("<div class='webStatusWrap'>"));
    GP.LED(ledId, state);
    GP.SEND(F("</div>"));
}

static void webControlsTableBegin()
{
    GP.TABLE_BEGIN("28%,60%,12%", GP_ALS(GP_LEFT, GP_RIGHT, GP_CENTER), "100%");
}

static void webControlsBtnCell(const char *openId, const char *closeId, const char *onText, const char *offText)
{
    GP.TD(GP_RIGHT);
    GP.SEND(F("<div class='webCtrlBtns'>"));
    GP.BUTTON_MINI(openId, onText, "", GP_GREEN, "44px", false, true);
    GP.BUTTON_MINI(closeId, offText, "", GP_GREEN, "44px", false, true);
    GP.SEND(F("</div>"));
}

static void webMotorRow(int i, const char *openId, const char *closeId, const char *ledId)
{
    char btnId[16], promptId[16], labelId[16];
    snprintf(btnId, sizeof(btnId), "WebBtnRenM%d", i + 1);
    snprintf(promptId, sizeof(promptId), "WebRenM%d", i + 1);
    snprintf(labelId, sizeof(labelId), "WebLblM%d", i + 1);
    bool isOpen = MotorDriver[i] && MotorDriver[i]->IsOpen();

    GP.TR();
    GP.TD(GP_LEFT);
    webNameCell(btnId, promptId, "Название мотора", labelId, data.MotorName[i]);
    webControlsBtnCell(openId, closeId, "Откр", "Закр");
    webStatusCell(ledId, isOpen);
}

static void webPumpRow()
{
    bool isOn = Pump && Pump->GetState();

    GP.TR();
    GP.TD(GP_LEFT);
    webNameCell("WebBtnRenPump", "WebRenPump", "Название насоса", "WebLblPump", data.PumpName);
    webControlsBtnCell("WebPumpOn", "WebPumpOff", "Вкл", "Выкл");
    webStatusCell("WebPumpLed", isOn);
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

static void webTempRow(const char *btnId, const char *promptId, const char *labelId, const char *name, const char *valueId, TSensor_DS18B20 *sensor)
{
    GP.TR();
    GP.TD(GP_LEFT);
    webNameCell(btnId, promptId, "Название датчика", labelId, name);
    webValueCell(valueId, formatTempC(sensor));
}

#if defined(ESP8266) || defined(ESP32)
static void webDashboardUpdate()
{
    if (ui.update("WebLblT1"))
        ui.answer(data.Temp1Name);
    if (ui.update("WebLblT2"))
        ui.answer(data.Temp2Name);
    if (ui.update("WebLblBarrel"))
        ui.answer(data.BarrelName);
    if (ui.update("WebLblM1"))
        ui.answer(data.MotorName[0]);
    if (ui.update("WebLblM2"))
        ui.answer(data.MotorName[1]);
    if (ui.update("WebLblM3"))
        ui.answer(data.MotorName[2]);
    if (ui.update("WebLblM4"))
        ui.answer(data.MotorName[3]);
    if (ui.update("WebLblPump"))
        ui.answer(data.PumpName);

    if (ui.update("WebTemp1"))
        ui.answer(formatTempC(Temp1));
    if (ui.update("WebTemp2"))
        ui.answer(formatTempC(Temp2));
    if (ui.update("WebBarrelLed"))
        ui.answer(BarrelIsFull() ? 1 : 0);
    for (int i = 0; i < 4; i++)
    {
        char ledId[16];
        snprintf(ledId, sizeof(ledId), "WebMotor%dLed", i + 1);
        if (ui.update(ledId))
            ui.answer(MotorDriver[i] && MotorDriver[i]->IsOpen() ? 1 : 0);
    }
    if (ui.update("WebPumpLed"))
        ui.answer(Pump && Pump->GetState() ? 1 : 0);
    if (ui.update("WebMqttLed"))
        ui.answer(AppMQTT && AppMQTT->IsMQTTConnected() ? 1 : 0);
}
#endif

static void webBarrelRow()
{
    GP.TR();
    GP.TD(GP_LEFT);
    webNameCell("WebBtnRenBarrel", "WebRenBarrel", "Название датчика", "WebLblBarrel", data.BarrelName);
    webStatusCell("WebBarrelLed", BarrelIsFull());
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

static bool webHandleRenameClick(const char *promptId, char *dst)
{
    if (!ui.click() || !ui.click(promptId))
        return false;

    String val = ui.clickText();
    val.trim();
    if (val.length() == 0)
        return false;
    if (val.length() >= WEB_LABEL_LEN)
        val = val.substring(0, WEB_LABEL_LEN - 1);

    webCopyStr(dst, val.c_str(), WEB_LABEL_LEN);
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

void build()
{
    GP.BUILD_BEGIN(GP_DARK, 1600);

#if defined(ESP8266) || defined(ESP32)
    GP.UPDATE(
        "WebLblT1,WebLblT2,WebLblBarrel,WebLblM1,WebLblM2,WebLblM3,WebLblM4,WebLblPump,"
        "WebTemp1,WebTemp2,WebBarrelLed,WebMotor1Led,WebMotor2Led,WebMotor3Led,WebMotor4Led,WebPumpLed,WebMqttLed",
        2000);
    GP.UPDATE_CLICK(
        "WebTemp1,WebTemp2,WebBarrelLed,WebMotor1Led,WebMotor2Led,WebMotor3Led,WebMotor4Led,WebPumpLed,WebMqttLed",
        "WebMotor1Open,WebMotor1Close,WebMotor2Open,WebMotor2Close,WebMotor3Open,WebMotor3Close,WebMotor4Open,WebMotor4Close,WebPumpOn,WebPumpOff");
    GP.RELOAD_CLICK("WebDashRefresh");
#endif

    GP.GRID_BEGIN(1600);
    GP.GRID_RESPONSIVE(768);
    webAppLayoutStyle();

    GP.BLOCK_TAB_BEGIN("Панель");

    webPanelStyle();
    GP.SEND(F("<div class='webPanel'>"));

    GP.LABEL("Датчики", "", GP_DEFAULT, 0, true);
    webActuatorTableBegin();
   // webActuatorTableHeader("", "");
    webTempRow("WebBtnRenT1", "WebRenT1", "WebLblT1", data.Temp1Name, "WebTemp1", Temp1);
    webTempRow("WebBtnRenT2", "WebRenT2", "WebLblT2", data.Temp2Name, "WebTemp2", Temp2);
    webBarrelRow();
    GP.TABLE_END();
    GP.HR();

    GP.LABEL("Моторы", "", GP_DEFAULT, 0, true);
    webControlsTableBegin();
    webMotorRow(0, "WebMotor1Open", "WebMotor1Close", "WebMotor1Led");
    webMotorRow(1, "WebMotor2Open", "WebMotor2Close", "WebMotor2Led");
    webMotorRow(2, "WebMotor3Open", "WebMotor3Close", "WebMotor3Led");
    webMotorRow(3, "WebMotor4Open", "WebMotor4Close", "WebMotor4Led");
    GP.TABLE_END();

    GP.HR();
    GP.LABEL("Насос", "", GP_DEFAULT, 0, true);
    webControlsTableBegin();
    webPumpRow();
    GP.TABLE_END();

    GP.SEND(F("</div>"));

    GP.BLOCK_END();

    GP_MAKE_BLOCK_TAB("Wi-Fi",
                      GP.PLAIN("После смены SSID/пароля нажмите «Перезагрузить».");
                      GP.BREAK();
                      GP_MAKE_BOX(
                          GP.LABEL("SSID");
                          GP.TEXT("WiFiSSID", "", data.WiFiSSID););

                      GP_MAKE_BOX(
                          GP.LABEL("Пароль");
                          GP.PASS("WiFiPass", "", data.WiFiPassword, "", 63);););

    GP.BLOCK_TAB_BEGIN("MQTT");

    GP_MAKE_BOX(
        GP.LABEL("Cервер");
        GP.TEXT("MQTTServer", "", data.MQTTServer););

    GP_MAKE_BOX(
        GP.LABEL("Порт");
        GP.NUMBER("MQTTPort", "", data.Port));

    GP_MAKE_BOX(
        GP.LABEL("Топик");
        GP.TEXT("MQTTTopic", "", data.MQTTTopic););

#if defined(ESP8266) || defined(ESP32)
    GP.BOX_BEGIN(GP_LEFT, "100%");
    GP.LABEL("MQTT: ");
    GP.LED("WebMqttLed", AppMQTT && AppMQTT->IsMQTTConnected());
    GP.BOX_END();
#endif

    GP.BOX_BEGIN(GP_LEFT, "100%");
    GP.BUTTON("RebootBtn", "Перезагрузить");
    GP.BUTTON("SaveBtn", "Сохранить");
    GP.BUTTON_MINI("WebDashRefresh", "Обновить", "", GP_GRAY, "", false, true);
    GP.BOX_END();

    GP.BLOCK_END();

    GP.GRID_END();

    GP.BUILD_END();
}

void action()
{
#if defined(ESP8266) || defined(ESP32)
    if (ui.update())
    {
        webDashboardUpdate();
        return;
    }
#endif

    if (ui.click())
    {
        if (webHandleRenameClick("WebRenT1", data.Temp1Name))
            return;
        if (webHandleRenameClick("WebRenT2", data.Temp2Name))
            return;
        if (webHandleRenameClick("WebRenBarrel", data.BarrelName))
            return;
        if (webHandleRenameClick("WebRenM1", data.MotorName[0]))
            return;
        if (webHandleRenameClick("WebRenM2", data.MotorName[1]))
            return;
        if (webHandleRenameClick("WebRenM3", data.MotorName[2]))
            return;
        if (webHandleRenameClick("WebRenM4", data.MotorName[3]))
            return;
        if (webHandleRenameClick("WebRenPump", data.PumpName))
            return;

        if (ui.click("WebMotor1Open"))
            MotorDriver[0]->Open();
        else if (ui.click("WebMotor1Close"))
            MotorDriver[0]->Close();
        else if (ui.click("WebMotor2Open"))
            MotorDriver[1]->Open();
        else if (ui.click("WebMotor2Close"))
            MotorDriver[1]->Close();
        else if (ui.click("WebMotor3Open"))
            MotorDriver[2]->Open();
        else if (ui.click("WebMotor3Close"))
            MotorDriver[2]->Close();
        else if (ui.click("WebMotor4Open"))
            MotorDriver[3]->Open();
        else if (ui.click("WebMotor4Close"))
            MotorDriver[3]->Close();
        else if (ui.click("WebPumpOn"))
            Pump->On();
        else if (ui.click("WebPumpOff"))
            Pump->Off();

        ui.clickStr("MQTTServer", data.MQTTServer);
        ui.clickStr("MQTTTopic", data.MQTTTopic);
        ui.clickInt("MQTTPort", data.Port);
        ui.clickStr("WiFiSSID", data.WiFiSSID);
        ui.clickStr("WiFiPass", data.WiFiPassword);

        if (ui.click("SaveBtn"))
        {

            preferences.begin("config", false);

            preferences.putInt("Port", data.Port);
            preferences.putString("Server", data.MQTTServer);
            preferences.putString("Topic", data.MQTTTopic);
            preferences.putString("wifi_ssid", data.WiFiSSID);
            preferences.putString("wifi_pass", data.WiFiPassword);

            preferences.end();

#if defined(ESP8266) || defined(ESP32)
            if (AppWiFi)
                AppWiFi->ApplySettingsFromNvs();
#endif

            if (strlen(data.MQTTServer) > 0)
            {
                String ttopic = String(data.MQTTTopic);
                if (ttopic.length() == 0)
                    ttopic = "teplica";
                AppMQTT->InitMQTT(String(data.MQTTServer), data.Port, ttopic);
            }
            else
                AppMQTT->MqttShutdown();
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
