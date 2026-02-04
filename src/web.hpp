// Based upon https://github.com/LearnEmbeddedSystems/pico-w-webserver-template and official pico_w access point example
#ifndef WEB_H
#define WEB_H

#include "common.hpp"
#include <WiFi.h>
#include <WifiMulti.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include "control.hpp"
#include "sd.hpp"

WebServer server(80);
bool modeAP=false;
uint numSSIDs=0;
WiFiUDP udp;
char beaconMessage[BEACON_BUFFER]={'\0'};
IPAddress beaconTarget;

String readLine(File f) {
  String res="";
  char c = f.read();
  while (c!=-1 && c!='\n') {
    if (c!='\r') {
      res+=c;
    }
  }
  return res;
}

bool readNet(File f, char* ssid, char * pwd) {
  ssid[0]='\0';
  pwd[0]='\0';
  char *buf=ssid;
  int c = f.read();
  bool ret = false;
  while (c!=-1 && c!='\n') {
    if (c!='\r') {
      if (c=='\t') {
        buf[0]='\0';
        buf=pwd;
        ret=true;
      } else {
        buf[0]=c;
        buf++;
      }
    }
    c=f.read();
  }
  buf[0]='\0';
  return ret;
}

bool trySavedWifi() {
  WiFiMulti wm;
  File f=SDFS.open(WIFI_FILE, "r");
  
  if (!f) return false;
  char ssid[SSID_LENGTH+1], pwd[SSID_LENGTH+1];
  while (readNet(f,ssid,pwd)) {
    wm.addAP(ssid, pwd);
  }
  f.close();
  uint8_t ret=wm.run();
  return (ret == WL_CONNECTED);
}

void scanAPs() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  numSSIDs=WiFi.scanNetworks();
}

//------------------------------------------------------------------------------
String buildDebugMsg(String m) {
  String message = m;
  message += "\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += server.method();
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i <server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  return message;
}
//------------------------------------------------------------------------------
void handleNotFound() {
  server.send(404, "text/plain", buildDebugMsg("FileNotFound"));  
}
//------------------------------------------------------------------------------
bool loadFromSD(String path)
{
  String dataType = "text/html";

  Serial.print("Requested page -> ");
  Serial.println(path);
  if (SDFS.exists(path))
  {
    File page = SDFS.open(path.c_str(), "r");
    if (!page)
    {
      handleNotFound();
      return false;
    }

    server.streamFile(page, "text/html");

    page.close();
  }
  else
  {
    handleNotFound();
    return false;
  }
  return true;
}
//------------------------------------------------------------------------------
void handleRoot()
{
  loadFromSD("/index.html");
}
//------------------------------------------------------------------------------
void handleSet() {
  server.send(200, "text/plain","Not implemented");
}
//------------------------------------------------------------------------------
void handleCommand() {
    for (int a=0; a<server.args();a++) {
        String command=server.argName(a);
        if (command=="move") {
            double rev=server.arg(a).toDouble();
            motors.forwardBy(rev);
        } else if (command=="turn") {
            double rev=server.arg(a).toDouble();
            motors.panBy(rev);
        } else if (command=="pen") {
            int deg = server.arg(a).toInt();
            serv.moveTo(deg);
        }
    }
}
//------------------------------------------------------------------------------
void initWeb();

void handleConnect() {
  if (!server.hasArg("SSID") || !server.hasArg("PWD")) {
    server.send(400, "text/plain","Invalid arguements");
    return;
  }
  File f=SDFS.open(WIFI_FILE, "a");
  if (!f) {
    server.send(500, "text/plain", "Can't update Wifi details");
    return;
  }
  f.print(server.arg("SSID"));
  f.print("\t");
  f.print(server.arg("PWD"));
  f.print("\n");
  f.close();
  server.send(200, "text/plain", "OK");
  delay(250);
  WiFi.disconnect();
  server.stop();
  delay(250);
  initWeb();
}
//------------------------------------------------------------------------------
void handleAPs() {
  if (!modeAP) {
    server.send(200, "application/json", "[]");
    return;
  }
  String res="[";
  for (int i=0; i<numSSIDs; i++) {
    if (i>0) res+=',';
    res+='"';
    res+=WiFi.SSID(i);
    res+='"';
  }
  res+=']';
  Serial.println(res);
  server.send(200,"application/json", res);
}
//------------------------------------------------------------------------------
double lastBeacon = (double)to_ms_since_boot(get_absolute_time());

bool nextBeacon() {
  return (double)to_ms_since_boot(get_absolute_time())-lastBeacon > BEACON_INTERVAL_MS;
}

void resetBeacon() {
  lastBeacon = (double)to_ms_since_boot(get_absolute_time());
}


void handleBeacon() {
  if (nextBeacon()) {
    udp.beginPacketMulticast(beaconTarget, BEACON_PORT, WiFi.localIP());
    udp.write(beaconMessage);
    udp.endPacket();
    resetBeacon();
  }
}
//------------------------------------------------------------------------------
void initWeb() {

  modeAP=!trySavedWifi();
  if (modeAP) {
    scanAPs();
    WiFi.beginAP(DEF_SSID, DEF_PWD);

    Serial.print("Connecting...\n");
    int status = WiFi.status();
    while (status != WL_CONNECTED)
    {
      Serial.print("Status:");
      Serial.println(status);
      delay(500);
      status = WiFi.status();
    }
  }
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("UDP Broadcast Port:");
  Serial.println(BEACON_PORT);
  sprintf(beaconMessage,"Panel IP Address: %s\0", WiFi.localIP().toString());
  beaconTarget.fromString(BEACON_TARGET);
  
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/command", handleCommand);
  server.on("/connect", handleConnect);
  server.on("/APs", handleAPs);
  server.on("/test", []() {
    server.send(200, "text/plain", buildDebugMsg("Test OK"));
  });
  server.onNotFound(handleNotFound);

  server.begin();

}
//------------------------------------------------------------------------------
void processWeb() {
    server.handleClient();
    handleBeacon();
}
#endif