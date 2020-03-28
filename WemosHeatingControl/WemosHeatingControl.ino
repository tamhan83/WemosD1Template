#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>

#include <Adafruit_NeoPixel.h>
#define LED_COUNT 4
#define LED_PIN 2   
Adafruit_NeoPixel strip(LED_COUNT, 2, NEO_GRB + NEO_KHZ800);
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#ifndef STASSID
#define STASSID "Doghouse_2.4"
#define STAPSK  "GuitarHero40!"
#endif

//#define topicBoiler "homie/homey-1/boiler-pump1/onoff"
//#define topicPumpDownstairs "homie/homey-1/boiler-pump1/onoff1"
//#define topicPumpUpstairs "homie/homey-1/pump---upstairs/onoff"
//#define topicImageHeater "homie/homey-1/image-heater/onoff"
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

    //Set up LED strip
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    //strip.setBrightness(10); // Set BRIGHTNESS to about 1/5 (max = 255)
    Serial.println("Led setup");

    strip.setPixelColor(0, strip.Color(1, 1, 1));         //  Set pixel's color (in RAM)
    strip.setPixelColor(1, strip.Color(1, 1, 1));         //  Set pixel's color (in RAM)
    strip.setPixelColor(2, strip.Color(1, 1, 1));         //  Set pixel's color (in RAM)
    strip.setPixelColor(3, strip.Color(1, 1, 1));         //  Set pixel's color (in RAM)
    strip.setBrightness(0);
    strip.show();
}


void loop() {
    //MQTT

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

    checkStatusLight(topic, payload, length, topicBoiler, 0);
    checkStatusLight(topic,  payload, length, topicPumpDownstairs, 1);
    checkStatusLight(topic, payload, length, topicPumpUpstairs, 2);
    checkStatusLight(topic, payload, length, topicImageHeater, 3);

}

void checkStatusLight(char* topic, byte* payload, unsigned int length, char* topicToCheck, int led) {
    if (strcmp(topic, topicToCheck) == 0)
    {

        //memcpy
        if (!strncmp((char*)payload, "ON", length))
        {
            strip.setBrightness(2);
            strip.show();                          //  Update strip to match
            strip.setPixelColor(led, strip.Color(100, 100, 1));         //  Set pixel's color (in RAM)
            strip.show();
        }
        else
        {
            strip.setPixelColor(led, strip.Color(0, 0, 0));         //  Set pixel's color (in RAM)
            //strip.setBrightness(0);
            strip.show();                          //  Update strip to match
        }

    }
}
