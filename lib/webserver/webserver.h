#pragma once

#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include <ArduinoOTA.h>
#include <Update.h>

#include "html_strings.h"

typedef void(WebserverConnectedCb(void *arg));

// TODO ugly way to pass variable but I'm tired
void webserver_start(std::vector<float> *readings, std::vector<long> *epocTime,
                     float *var, WebserverConnectedCb *cb);