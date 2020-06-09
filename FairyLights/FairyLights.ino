#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>



#ifndef STASSID
#define STASSID "Doghouse_2.4"
#define STAPSK  "GuitarHero40!"
#endif

bool allowOTA = false;

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

//MQTT
const char* mqtt_server = "192.168.1.13";

WiFiClient espClient;
PubSubClient client(espClient);


// constants won't change. They're used here to
// set pin numbers:
const int buttonPin = 12;     // the number of the pushbutton pin
const int ledPin = 14;      // the number of the LED pin

bool lightsOn = false;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
void setup() {
    Serial.begin(115200);
    Serial.println("Booting");

    Serial.println("hi");
    pinMode(ledPin, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);
    digitalWrite(ledPin, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);


    ///////////////////////
    Serial.begin(115200);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    //MQTT
    client.setServer(mqtt_server, 1883);
    //client.setCallback(callback);


    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        }
        else { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
        });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
        });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        }
        else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
        });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    

    server.on("/enableOta", handleEnableOTA);

    server.begin();
    Serial.println("HTTP server started");
}
void loop() {
    

    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW)
    {
        if (lightsOn == false) {
            // turn LED on:
            digitalWrite(ledPin, HIGH);
            lightsOn = true;
            client.publish("Custom/FairyLights", "ON");
        }
        else        
        {
            digitalWrite(ledPin, false);
            lightsOn = false;
            client.publish("Custom/FairyLights", "OFF");
        }
        delay(1000);
    }
    // Added the delay so that we can see the output of button
    delay(50);

    //infrastructure


    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}

void handleEnableOTA() {
    Serial.println("Enable OTA");
    server.send(200, "text/plain", "OTA enabled");
    allowOTA = true;
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("outTopic", "hello world");
            // ... and resubscribe
            //client.subscribe("inTopic");
            //client.subscribe(topicPumpDownstairs);
            //client.publish("Wemos", WiFi.localIP());

            String base = "Wemos/FairyLights/";
            base.concat(WiFi.localIP().toString());
            char topic[40];
            base.toCharArray(topic, 40);

            client.publish(topic, "connected");
        }
        else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}