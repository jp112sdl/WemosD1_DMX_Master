bool doWifiConnect() {

  String _ssid = WiFi.SSID();
  String _psk = WiFi.psk();

  Serial.println("SSID = " + _ssid);

  const char* IPStr = WemosNetConfig.IP; byte IPBytes[4]; parseBytes(IPStr, '.', IPBytes, 4, 10);
  const char* NETMASKStr = WemosNetConfig.NETMASK; byte NETMASKBytes[4]; parseBytes(NETMASKStr, '.', NETMASKBytes, 4, 10);
  const char* GWStr = WemosNetConfig.GW; byte GWBytes[4]; parseBytes(GWStr, '.', GWBytes, 4, 10);

  if (!startWifiManager && _ssid != "" && _psk != "" ) {
    Serial.println("Connecting WLAN the classic way...");
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _psk.c_str());
    int waitCounter = 0;
    if (String(WemosNetConfig.IP) != "0.0.0.0")
      WiFi.config(IPAddress(IPBytes[0], IPBytes[1], IPBytes[2], IPBytes[3]), IPAddress(GWBytes[0], GWBytes[1], GWBytes[2], GWBytes[3]), IPAddress(NETMASKBytes[0], NETMASKBytes[1], NETMASKBytes[2], NETMASKBytes[3]));

    while (WiFi.status() != WL_CONNECTED) {
      waitCounter++;
      Serial.print(".");
      if (waitCounter == 20) {
        return false;
      }
      delay(500);
    }
    Serial.println("\nWifi Connected");
    printWifiStatus();
    return true;
  } else {
    WiFiManager wifiManager;
    digitalWrite(LED_BUILTIN, LOW);
    wifiManager.setDebugOutput(wifiManagerDebugOutput);
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    WiFiManagerParameter custom_UDP("custom_UDP", "UDP Port", WemosNetConfig.UDPPORT, IPSIZE);

    WiFiManagerParameter custom_IP("custom_IP", "IP-Adresse", (String(WemosNetConfig.IP) != "0.0.0.0") ? WemosNetConfig.IP : "", IPSIZE);
    WiFiManagerParameter custom_NETMASK("custom_NETMASK", "Netzmaske",  (String(WemosNetConfig.NETMASK) != "0.0.0.0") ? WemosNetConfig.NETMASK : "", IPSIZE);
    WiFiManagerParameter custom_GW("custom_GW", "Gateway",  (String(WemosNetConfig.GW) != "0.0.0.0") ? WemosNetConfig.GW : "", IPSIZE);
    WiFiManagerParameter custom_text("<br/><br>Statische IP (wenn leer, dann DHCP):");

    wifiManager.addParameter(&custom_UDP);
    wifiManager.addParameter(&custom_text);
    wifiManager.addParameter(&custom_IP);
    wifiManager.addParameter(&custom_NETMASK);
    wifiManager.addParameter(&custom_GW);

    String Hostname = "ESP-DMX-" + WiFi.macAddress();

    wifiManager.setConfigPortalTimeout(CONFIGPORTALTIMEOUT);

    if (startWifiManager == true) {
      if (_ssid == "" || _psk == "" ) {
        wifiManager.resetSettings();
      }
      else {
        if (!wifiManager.startConfigPortal(Hostname.c_str())) {
          Serial.println("failed to connect and hit timeout");
          delay(1000);
          ESP.restart();
        }
      }
    }

    wifiManager.setSTAStaticIPConfig(IPAddress(IPBytes[0], IPBytes[1], IPBytes[2], IPBytes[3]), IPAddress(GWBytes[0], GWBytes[1], GWBytes[2], GWBytes[3]), IPAddress(NETMASKBytes[0], NETMASKBytes[1], NETMASKBytes[2], NETMASKBytes[3]));

    wifiManager.autoConnect(Hostname.c_str());

    Serial.println("Wifi Connected");
    Serial.println("CUSTOM STATIC IP: " + String(WemosNetConfig.IP) + " NETMASK: " + String(WemosNetConfig.NETMASK) + " GW: " + String(WemosNetConfig.GW));
    if (shouldSaveConfig) {
      if (String(custom_IP.getValue()).length() > 5) {
        Serial.println("Custom IP Address is set!");
        strcpy(WemosNetConfig.IP, custom_IP.getValue());
        strcpy(WemosNetConfig.NETMASK, custom_NETMASK.getValue());
        strcpy(WemosNetConfig.GW, custom_GW.getValue());

      } else {
        strcpy(WemosNetConfig.IP,      "0.0.0.0");
        strcpy(WemosNetConfig.NETMASK, "0.0.0.0");
        strcpy(WemosNetConfig.GW,      "0.0.0.0");
      }

      strcpy(WemosNetConfig.UDPPORT, custom_UDP.getValue());

      saveSystemConfig();

      delay(100);
      ESP.restart();
    }
    return true;
  }
}


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("AP-Modus ist aktiv!");
}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);
    str = strchr(str, sep);
    if (str == NULL || *str == '\0') {
      break;
    }
    str++;
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress IP = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(IP);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

