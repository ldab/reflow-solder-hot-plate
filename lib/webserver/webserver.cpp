#include "webserver.h"

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncEventSource events("/events"); // event source (Server-Sent events)
TimerHandle_t wifiReconnectTimer;

std::vector<float> _readings;
std::vector<long> _epocTime;

String processor(const String &var);

class CaptiveRequestHandler : public AsyncWebHandler
{
  public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    // request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request)
  {
    request->send_P(200, "text/html", HTTP_CONFIG, processor);
  }
};

void onFire(AsyncWebServerRequest *request, void *cb)
{
  struct reflow_profile {
    uint32_t preheat_temp;
    uint32_t preheat_rate;
    uint32_t soak_temp;
    uint32_t soak_rate;
    uint32_t reflow_temp;
    uint32_t reflow_time;
  };

  static struct reflow_profile profile;

  int params = request->params();

  for (int i = 0; i < params; i++) {
    AsyncWebParameter *p = request->getParam(i);
    log_d("%s: %d", p->name(), p->value().toInt());
    if (p->isPost()) {
      if (p->name() == "s00")
        profile.preheat_temp = p->value().toInt();
      if (p->name() == "s01")
        profile.preheat_rate = p->value().toInt();
      if (p->name() == "s10")
        profile.soak_temp = p->value().toInt();
      if (p->name() == "s11")
        profile.soak_rate = p->value().toInt();
      if (p->name() == "s20")
        profile.reflow_temp = p->value().toInt();
      if (p->name() == "22")
        profile.preheat_rate = p->value().toInt();
    }
  }

  // *cb(&profile);
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index,
              uint8_t *data, size_t len, bool final)
{
  if (!index) {
    log_d("Update Start: %s\n", filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  }
  log_d("Progress: %u of %u\r", Update.progress(), Update.size());
  if (!Update.hasError()) {
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
    }
  }
  if (final) {
    if (Update.end(true)) {
      log_d("Update Success: %uB\n", index + len);
      request->redirect("/");
      vTaskDelay(pdMS_TO_TICKS(500));
      ESP.restart();
    } else {
      Update.printError(Serial);
    }
  }
}

void onRequest(AsyncWebServerRequest *request)
{
  // Handle Unknown Request
  request->send(404, "text/plain", "OUCH");
}

String processor(const String &var)
{
  if (var == "CSS_TEMPLATE")
    return FPSTR(HTTP_STYLE);
  if (var == "INDEX_JS")
    return FPSTR(HTTP_JS);
  if (var == "HTML_HEAD_TITLE")
    return FPSTR(HTML_HEAD_TITLE);
  if (var == "HTML_INFO_BOX") {
    String ret = "";
    if (WiFi.isConnected()) {
      ret = "<strong> Connected</ strong> to ";
      ret += WiFi.SSID();
      ret += "<br><em><small> with IP ";
      ret += WiFi.localIP().toString();
      ret += "</small>";
    } else
      ret = "<strong> Not Connected</ strong>";
    return ret;
  }
  if (var == "HTML_CONFIG_SSID")
    return WiFi.SSID();
  if (var == "UPTIME") {
    String ret = String(millis() / 1000 / 60);
    ret += " min ";
    ret += String((millis() / 1000) % 60);
    ret += " sec";
    return ret;
  }
  if (var == "CHIP_ID") {
    String ret = String((uint32_t)ESP.getEfuseMac());
    return ret;
  }
  if (var == "FREE_HEAP") {
    String ret = String(ESP.getFreeHeap());
    ret += " bytes";
    return ret;
  }
  if (var == "SKETCH_INFO") {
    //%USED_BYTES% / &FLASH_SIZE&<br><progress value="%USED_BYTES%"
    // max="&FLASH_SIZE&">
    String ret = String(ESP.getSketchSize());
    ret += " / ";
    ret += String(ESP.getFlashChipSize());
    ret += "<br><progress value=\"";
    ret += String(ESP.getSketchSize());
    ret += "\" max=\"";
    ret += String(ESP.getFlashChipSize());
    ret += "\">";
    return ret;
  }
  if (var == "HOSTNAME")
    return String(WiFi.getHostname());
  if (var == "MY_MAC")
    return WiFi.macAddress();
  if (var == "MY_RSSI")
    return String(WiFi.RSSI());
  if (var == "FW_VER")
    return VERSION;
  if (var == "SDK_VER")
    return String(ESP_ARDUINO_VERSION_MAJOR) + "." +
           String(ESP_ARDUINO_VERSION_MINOR) + "." +
           String(ESP_ARDUINO_VERSION_PATCH);
  if (var == "ABOUT_DATE") {
    String ret = String(__DATE__) + " " + String(__TIME__);
    return ret;
  }
  if (var == "GRAPH_DATA" && _readings.size()) {
    String graphString;
    graphString.reserve(_readings.size() * 2);
    graphString = "[";
    for (size_t i = 0; i < _readings.size() - 1; i++) {
      graphString += "[";
      graphString += String(_epocTime[i]);
      graphString += ",";
      graphString += String(_readings[i], 0);
      graphString += "]";
      graphString += ",";
    }
    graphString += "[";
    graphString += String(_epocTime[_readings.size() - 1]);
    graphString += ",";
    graphString += String(_readings[_readings.size() - 1], 0);
    graphString += "]";
    graphString += "]";
    return graphString;
  }

  return String();
}

void configServer()
{
  server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssid, pass;
    int params = request->params();
    for (int i = 0; i < params; i++) {
      AsyncWebParameter *p = request->getParam(i);
      if (p->isPost()) {
        // HTTP POST ssid value
        if (p->name() == "ssid") {
          ssid = p->value().c_str();
          log_d("SSID set to: %s\n", ssid.c_str());
        }
        if (p->name() == "pass") {
          pass = p->value().c_str();
          log_d("Password set to: %s\n", pass.c_str());
        }
      }
    }

    WiFi.persistent(true);
    WiFi.begin(ssid.c_str(), pass.c_str());
    log_d("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(100);
    }
    log_d("Connected\n");
    request->redirect("http://" + WiFi.localIP().toString());
    vTaskDelay(pdMS_TO_TICKS(500));
    ESP.restart();
  });
}

void otaInit()
{
  ArduinoOTA.setHostname(HOSTNAME);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    log_i("Start updating %s", type.c_str());
  });
  ArduinoOTA.onEnd([]() { log_i("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    log_d("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    log_d("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      log_i("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      log_i("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      log_i("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      log_i("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      log_i("End Failed");
    }
  });

  ArduinoOTA.begin();
}

static void TaskWebserver(void *pvParameters)
{
  float *temp = (float *)pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    char msg[16];
    sprintf(msg, "%.1f", *temp);
    events.send(msg, "temperature");
    log_v("send event %s", msg);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  vTaskDelete(NULL);
}

static void TaskMDns(void *pvParameters)
{
  float *temp = (float *)pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    if (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA) {
      dnsServer.processNextRequest();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  vTaskDelete(NULL);
}

void webserver_start(std::vector<float> *readings, std::vector<long> *epocTime,
                     float *var, WebserverConnectedCb *cb)
{
  // 1440 samples, every 1 min = 24 hours
  _readings = *readings;
  _epocTime = *epocTime;

  _readings.reserve(1440);
  _epocTime.reserve(1440);

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);
  WiFi.begin();

  if (WiFi.waitForConnectResult() == WL_DISCONNECTED ||
      WiFi.waitForConnectResult() == WL_NO_SSID_AVAIL) { //~ 100 * 100ms
    log_d("WiFi Failed!: %u\n", WiFi.status());

    configServer();

    WiFi.softAP(HOSTNAME);

    server.onNotFound(
        [](AsyncWebServerRequest *request) { request->redirect("/"); });
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());

    log_d("Start Captive Portal at: %s\n", WiFi.softAPIP().toString().c_str());

    server.addHandler(new CaptiveRequestHandler())
        .setFilter(ON_AP_FILTER); // only when requested from AP
  } else {
    log_d("WiFi Connected, IP: %s\n", WiFi.localIP().toString().c_str());

    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "0.pool.ntp.org",
                 "1.pool.ntp.org");

    server.addHandler(&events);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", HTTP_INDEX, processor);
    });

    server.on("/", HTTP_POST, [cb](AsyncWebServerRequest *request) {
      onFire(request, (void *)cb);
      cb();
      request->send_P(200, "text/html", HTTP_INDEX, processor);
      events.send("Heating", "display");
    });

    server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", HTTP_SETUP, processor);
    });

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", HTTP_CONFIG, processor);
    });

    configServer();

    server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", HTTP_INFO, processor);
    });

    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->redirect("/");
      vTaskDelay(pdMS_TO_TICKS(500));
      ESP.restart();
    });

    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", HTTP_UPDATE, processor);
    });

    server.on(
        "/update", HTTP_POST, [](AsyncWebServerRequest *request) {}, onUpload);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      time_t epoc = mktime(&timeinfo);
      _epocTime.push_back((long)epoc);
      _readings.push_back(*var);
      log_d("_readings.size(): %u\n", _readings.size());
      log_d("epoc: %u\n", epoc);
    }
  }
  server.onNotFound(onRequest);
  server.begin();

  xTaskCreate(TaskWebserver, "TaskWebserver", 4096, var, 2, NULL);
  xTaskCreate(TaskMDns, "TaskMDns", 4096, NULL, 1, NULL);
}