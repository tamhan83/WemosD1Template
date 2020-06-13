#include <PubSubClient.h>
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


const char* device_name = "garden";

//MQTT
const char* mqtt_server = "192.168.1.13";
const char* topicEnableOta = "wemos/heart_lights/enable_ota/set";
const char* topicToggle = "wemos/heart_lights/toggle";
const char* topicSetStatus = "wemos/heart_lights/set";


WiFiClient espClient;
PubSubClient client(espClient);

// constants won't change. They're used here to
// set pin numbers:
const int buttonPin = 12;     // the number of the pushbutton pin
const int ledPin = 0;      // the number of the LED pin

bool lightsOn = true;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
void setup() {
    Serial.begin(115200);
    Serial.println("Booting");

    Serial.println("hi");
    pinMode(ledPin, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);
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
    client.setCallback(callback);


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

    digitalWrite(ledPin, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
    
    Serial.println("HTTP server started");
}

void loop() {
    

    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW)
    {
        
        if (lightsOn == false) {
            // turn LED on:
            //Serial.println("Button turn on");
            digitalWrite(ledPin, HIGH);
            lightsOn = true;
            client.publish("wemos/heart_lights/state", "ON");
        }
        else        
        {
            //Serial.println("Button turn off");
            digitalWrite(ledPin, LOW);
            lightsOn = false;
            client.publish("wemos/heart_lights/state", "OFF");
        }
        delay(1000);
    }
    // Added the delay so that we can see the output of button
    delay(50);



    //infrastructure
    if (allowOTA)
    {
        ArduinoOTA.handle();
    }

    if (!client.connected()) {
        reconnect();
    }
    client.loop();
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
            // ... and resubscribe
            //client.subscribe("inTopic");
            client.subscribe(topicEnableOta);
            client.subscribe(topicToggle);
            client.subscribe(topicSetStatus);
            
            
            //client.publish("Wemos", WiFi.localIP());

            String base = "wemos/heart_lights/";
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

void callback(char* topic, byte* payload, unsigned int length) {
    //Serial.println("Message arrived");
    
    if (strcmp(topic, topicEnableOta) == 0)
    {        
        //Serial.println("Enable OTA");
        client.publish("wemos/heart_lights/enable_ota/state", "enabled");
        allowOTA = true;
    }
    if (strcmp(topic, topicToggle) == 0)
    {
        //Serial.println("Toggle");
        if (lightsOn == false)
        {
            digitalWrite(ledPin, HIGH);
            lightsOn = true;
        }
        else
        {
            lightsOn = false;
            digitalWrite(ledPin, LOW);
        }
    }
    if (strcmp(topic, topicSetStatus) == 0)
    {
        //Serial.println("SetStatus");
        //Serial.println(length);
        //Serial.println((char*)payload);
        if (strncmp((char*)payload, "ON", length) == 0)
        { 
            digitalWrite(ledPin, HIGH);
            lightsOn = true;
            client.publish("wemos/heart_lights/state", "ON");
        }
        if (strncmp((char*)payload, "OFF", length) == 0)
        {
            digitalWrite(ledPin, LOW);
            lightsOn = false;
            client.publish("wemos/heart_lights/state", "OFF");
        }
    }
}