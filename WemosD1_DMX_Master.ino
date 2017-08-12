#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <ESPDMX.h>

#define SWITCHPIN           D2

#define IPSIZE              16
#define CONFIGPORTALTIMEOUT 180

struct dmxparam_t {
  byte Channel = 0;
  byte Value = 0;
} DmxParam;

//String DmxParams[255]

//WifiManager - don't touch
bool shouldSaveConfig          = false;
const String configJsonFile = "config.json";
#define wifiManagerDebugOutput  true
bool startWifiManager = false;
struct wemosnetconfig_t {
  char IP[IPSIZE]      = "0.0.0.0";
  char NETMASK[IPSIZE] = "0.0.0.0";
  char GW[IPSIZE]      = "0.0.0.0";
  char UDPPORT[IPSIZE] = "6674";
} WemosNetConfig;

DMXESPSerial dmx;

struct udp_t {
  WiFiUDP UDP;
  char incomingPacket[255];
} UDPClient;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(SWITCHPIN, INPUT_PULLUP);

  Serial.println("\nConfig-Modus aktivieren?");
  for (int i = 0; i < 20; i++) {
    if (digitalRead(SWITCHPIN) == LOW) {
      startWifiManager = true;
      break;
    }
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }

  if (!loadSystemConfig()) startWifiManager = true;

  if (doWifiConnect()) {
    startOTAhandling();
    UDPClient.UDP.begin(atoi(WemosNetConfig.UDPPORT));
    dmx.init(512);
  } else ESP.restart();
}

void loop() {
  String paket = handleUDP();
  if (paket != "") {
    while (paket.indexOf(";") != -1) {
      String teil1 = paket.substring(0, paket.indexOf(";"));
      DmxParam.Channel = (teil1.substring(0, teil1.indexOf(","))).toInt();
      DmxParam.Value = (teil1.substring(teil1.indexOf(",") + 1, teil1.length())).toInt();
      paket = paket.substring(paket.indexOf(";") + 1, paket.length());
      dmx.write(DmxParam.Channel, DmxParam.Value);
      Serial.println("Steuerbefehl erhalten: Chan = " + String(DmxParam.Channel) + ", Val = " + String(DmxParam.Value));
    }
  }
  dmx.update();
}

String handleUDP() {
  int packetSize = UDPClient.UDP.parsePacket();
  if (packetSize) {
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, UDPClient.UDP.remoteIP().toString().c_str(), UDPClient.UDP.remotePort());
    int len = UDPClient.UDP.read(UDPClient.incomingPacket, 255);
    if (len > 0) {
      UDPClient.incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", UDPClient.incomingPacket);

    UDPClient.UDP.beginPacket(UDPClient.UDP.remoteIP(), UDPClient.UDP.remotePort());
    UDPClient.UDP.write("ACK");
    UDPClient.UDP.endPacket();

    String message = String(UDPClient.incomingPacket);
    message.trim();
    return message;
  } else return "";
}
