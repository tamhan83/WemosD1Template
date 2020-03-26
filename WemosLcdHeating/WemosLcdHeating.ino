#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 2
#define TFT_CS 15

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(15, 2);


#ifndef STASSID
#define STASSID "Doghouse_2.4"
#define STAPSK  "GuitarHero40!"
#endif

#define topicBoiler "heating/boiler"
#define topicPumpDownstairs "heating/pump_downstairs"
#define topicPumpUpstairs "heating/pump_upstairs"
#define topicImageHeater "heating/image-heater"


const char* ssid = STASSID;
const char* password = STAPSK;

//MQTT
const char* mqtt_server = "192.168.1.13";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

bool boilerOn = false;
bool pumpDownstairsOn = false;
bool pumpUpstairsOn = false;
bool imageHeaterOn = false;

bool statusChange = false;

void setup() {
    Serial.begin(115200);
    Serial.println("Booting");
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

    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("HTTP server started");
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW


    tft.begin();
    //tft.setRotation(0);
}


void loop() {
    //MQTT

    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    //delay(50);

    /*if (statusChange == true)
    {
        printStatus();
        statusChange = false;
    }*/

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
            client.subscribe("inTopic");
            client.subscribe(topicBoiler);
            client.subscribe(topicPumpDownstairs);
            client.subscribe(topicPumpUpstairs);
            client.subscribe(topicImageHeater);

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
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] '");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println("'");

    testText(payload, length);

    checkStatusLight(topic, payload, length, topicBoiler, &boilerOn);
    checkStatusLight(topic, payload, length, topicPumpDownstairs, &pumpDownstairsOn);
    checkStatusLight(topic, payload, length, topicPumpUpstairs, &pumpUpstairsOn);
    checkStatusLight(topic, payload, length, topicImageHeater, &imageHeaterOn);

    printStatus();
    
}

void printStatus()
{
    tft.fillScreen(ILI9341_BLACK);
    unsigned long start = micros();
    tft.setCursor(0, 0);
    
    
    tft.setTextColor(ILI9341_YELLOW);
    tft.setTextSize(2);
    tft.println("Hello World!");

    if(pumpDownstairsOn)
        tft.println("Down ON");
    else
        tft.println("Down OFF");

    if (pumpUpstairsOn)
        tft.println("Up: ON");
    else
        tft.println("Ups: OFF");
    

}

unsigned long testText(byte* payload, unsigned int length) {
    tft.fillScreen(ILI9341_BLACK);
    unsigned long start = micros();
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
    tft.println("Hello World!");
    tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
    for (int i = 0; i < length; i++) {
        tft.print((char)payload[i]);
    }

    return micros() - start;
}

void checkStatusLight(char* topic, byte* payload, unsigned int length, char* topicToCheck, bool* statusVar) {
    if (strcmp(topic, topicToCheck) == 0)
    {

        //memcpy
        if (!strncmp((char*)payload, "ON", length))
        {
            if (*statusVar == false)
                statusChange = true;
            *statusVar = true;
            
        }
        else
        {
            if (*statusVar == true)
                statusChange = true;

            *statusVar = false;
        }

    }
}