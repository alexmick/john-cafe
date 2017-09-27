#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266HTTPClient.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#include <SPI.h>
#include <MFRC522.h>

void playSound();
 
// NFC reader pin configuration
constexpr uint8_t RST_PIN = D2;
constexpr uint8_t SS_PIN = D8;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

int buzzPin = 5; // Buzzer pin

//define your default values here, if there are different values in config.json, they are overwritten.
char backend_server[200];

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("==== JOHN CAFE ====");

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("[INFO] Mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("[INFO] Mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("[INFO] Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\n[INFO] Successfully parsed json");

          strcpy(backend_server, json["backend_server"]);

        } else {
          Serial.println("[ERROR] Failed to load json config");
        }
      }
    }
  } else {
    Serial.println("[ERROR] Failed to mount FS");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_backend_server("server", "Backend server URL", backend_server, 200);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifiManager.addParameter(&custom_backend_server);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimum quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("JohnCafeAP", "password")) {
    Serial.println("[ERROR] Failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("[INFO] Connected to WiFi !");

  //read updated parameters
  strcpy(backend_server, custom_backend_server.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("[INFO] Saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["backend_server"] = backend_server;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("[ERROR] Failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
    Serial.println("\n[INFO] Config saved");
  }

  Serial.print("[INFO] Local ip : ");
  Serial.println(WiFi.localIP());

  Serial.println("[INFO] Starting SPI bus");
  SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details

}

void loop() {
  	// Look for new cards
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
    }

    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

    // HTTPClient http;

    // char uid[20];
    // http.begin();

    // int httpCode = http.GET();

    playSound();
}

void playSound() {
    tone(buzzPin, 660, 100);
	delay(150);
	tone(buzzPin, 660, 100);
	delay(300);
	tone(buzzPin, 660, 100);
	delay(300);
	tone(buzzPin, 510, 100);
	delay(100);
	tone(buzzPin, 660, 100);
	delay(300);
	tone(buzzPin, 770, 100);
	delay(550);
	tone(buzzPin, 380, 100);
	delay(575);
}