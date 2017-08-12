bool loadSystemConfig() {
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/" + configJsonFile)) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/" + configJsonFile, "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        Serial.println("Content of JSON Config-File: /"+configJsonFile);
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nJSON OK");
          ((json["IP"]).as<String>()).toCharArray(WemosNetConfig.IP, IPSIZE);
          ((json["NETMASK"]).as<String>()).toCharArray(WemosNetConfig.NETMASK, IPSIZE);
          ((json["GW"]).as<String>()).toCharArray(WemosNetConfig.GW, IPSIZE);
          ((json["UDPPORT"]).as<String>()).toCharArray(WemosNetConfig.UDPPORT, IPSIZE);
          
        } else {
          Serial.println("\nERROR loading config");
        }
      }
      return true;
    } else {
      Serial.println("/" + configJsonFile + " not found.");
      return false;
    }
    SPIFFS.end();
  } else {
    Serial.println("failed to mount FS");
    return false;
  }
}

bool saveSystemConfig() {
  SPIFFS.begin();
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["IP"] = WemosNetConfig.IP;
  json["NETMASK"] = WemosNetConfig.NETMASK;
  json["GW"] = WemosNetConfig.GW;
  json["UDPPORT"] = WemosNetConfig.UDPPORT;
  if (String(WemosNetConfig.UDPPORT) == "") strcpy(WemosNetConfig.UDPPORT,"6674");

  SPIFFS.remove("/" + configJsonFile);
  File configFile = SPIFFS.open("/" + configJsonFile, "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();

  SPIFFS.end();
}

