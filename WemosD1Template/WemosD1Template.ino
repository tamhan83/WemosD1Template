#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
ESP8266WiFiMulti WiFiMulti;
#include <ESP8266HTTPClient.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <Adafruit_NeoPixel.h>
#define LED_COUNT 61
#define LED_PIN 2   
Adafruit_NeoPixel strip(LED_COUNT, 2, NEO_GRB + NEO_KHZ800);
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#ifndef STASSID
#define STASSID "Doghouse_2.4"
#define STAPSK  "GuitarHero40!"
#endif

#define topicPumpDownstairs "homie/homey-1/boiler-pump1/onoff1"

bool allowOTA = false;
bool triggered = false;

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

//MQTT
const char* mqtt_server = "192.168.1.12";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void handleSetBrightness() {
    Serial.println("Set brightvess");
    //allowOTA = true;
String message = "";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      }
    server.send(200, "text/plain", message);
    Serial.println(message);

    int value = server.arg(0).toInt();
    strip.setBrightness(value);
    strip.show(); 
}

void handleEnableOTA() {
    Serial.println("Enable OTA");    
    server.send(200, "text/plain", "OTA enabled");
    allowOTA = true;
}


void handleSetColor() {
    Serial.println("Set Color ");    
    server.send(200, "text/plain", "Set Color");

    int red = server.arg(0).toInt();
    int green = server.arg(1).toInt();
    int blue = server.arg(2).toInt();
    
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(red, green, blue));
    }
    strip.show();
}

void handleRoot() {
    Serial.println("Incming request");
    server.send(200, "text/plain", "hello from esp8266!");
    digitalWrite(LED_BUILTIN, LOW);

    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    
    

    strip.setPixelColor(0, strip.Color(255, 0, 0));         //  Set pixel's color (in RAM)
    strip.setBrightness(40);
    strip.show();                          //  Update strip to match
    delay(1000);
    strip.setPixelColor(0, strip.Color(255, 200, 200));         //  Set pixel's color (in RAM)
    strip.setBrightness(40);
    strip.show();                          //  Update strip to match
    delay(1000);
}


void handleHallwayMotionDetected() {
    server.send(200, "text/plain", "motion");


    for (int i = 5; i < 200; i++)
    {
        strip.setBrightness(i);
        strip.show();
        delay(50);
    }

    delay(70000);

    for (int i = 200; i >= 5; i--)
    {
        strip.setBrightness(i);
        strip.show();
        delay(50);
    }
   
}


void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

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

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

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


    //Start web server
    if (MDNS.begin("esp8266")) {
        Serial.println("MDNS responder started");
    }

    server.on("/", handleRoot);
    server.on("/enableOta", handleEnableOTA);
    server.on("/setBrightness", handleSetBrightness);
    server.on("/setColor", handleSetColor);
    server.on("/hallwayMotionDetected", handleHallwayMotionDetected);
    
    
    server.begin();
    Serial.println("HTTP server started");
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW

    //Set up LED strip
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    //strip.setBrightness(10); // Set BRIGHTNESS to about 1/5 (max = 255)
    Serial.println("Led setup");
}

void checkButton() {
    // put your main code here, to run repeatedly:

    byte val = digitalRead(13);
    if (val == HIGH)
    {
        if (triggered == false)
        {
            Serial.print("Connected to ");
            triggered = true;

            if ((WiFiMulti.run() == WL_CONNECTED)) {

                WiFiClient client;

                HTTPClient http;

                Serial.print("[HTTP] begin...\n");
                if (http.begin(client, "http://192.168.1.13:1880/test?button=click")) {  // HTTP


                    Serial.print("[HTTP] GET...\n");
                    // start connection and send HTTP header
                    int httpCode = http.GET();
                }
            }

        }
    }
    else
    {
        if (triggered == true)
        {
            triggered = false;
        }
    }
    delay(100);
}

void loop() {
    if (allowOTA)
    {        
        ArduinoOTA.handle();
    }
    
    server.handleClient();
    MDNS.update();

    //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    //delay(100);                       // wait for a second
    //digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    //delay(1000);                       // wait for a second


    //MQTT

    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    /*unsigned long now = millis();
    if (now - lastMsg > 2000) {
        lastMsg = now;
        ++value;
        snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish("outTopic", msg);
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
            client.subscribe(topicPumpDownstairs);
            
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
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    if(strcmp(topic,topicPumpDownstairs))
    { 
        Serial.println(";Pump downstairs");
        strip.setPixelColor(0, strip.Color(255, 0, 0));         //  Set pixel's color (in RAM)
        strip.setBrightness(40);
        strip.show();                          //  Update strip to match
        delay(1000);
    }

    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
        digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
        // but actually the LED is on; this is because
        // it is active low on the ESP-01)
    }
    else {
        digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    }

}