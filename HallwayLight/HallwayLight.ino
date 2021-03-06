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
#include <ESP8266WebServer.h>

#include <Adafruit_NeoPixel.h>
#define LED_COUNT 20
#define LED_PIN 2   
Adafruit_NeoPixel strip(LED_COUNT, 2, NEO_GRB + NEO_KHZ800);
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif


#ifndef STASSID
#define STASSID "Doghouse_2.4"
#define STAPSK  "GuitarHero40!"
#endif

//String device_name = "hallway";

#define topicHallwayMotionDetected "wemos/hallway/motion_detected"

bool allowOTA = false;
bool triggered = false;

const char* ssid = STASSID;
const char* password = STAPSK;

int ledColor_R = 250;
int ledColor_G = 100;
int ledColor_B = 1;

ESP8266WebServer server(80);

//MQTT
const char* mqtt_server = "192.168.1.13";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void handleSetBrightness() {
    Serial.println("Set brightness");
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

    ledColor_R = server.arg(0).toInt();
    ledColor_G = server.arg(1).toInt();
    ledColor_B = server.arg(2).toInt();

    for (uint8_t i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(ledColor_R, ledColor_G, ledColor_B));
    }
    strip.show();
}

void handleSetColorRange() {
    Serial.println("Set Color Range");
    server.send(200, "text/plain", "Set Color");

    int start = server.arg(0).toInt();
    int end = server.arg(1).toInt();
    ledColor_R = server.arg(2).toInt();
    ledColor_G = server.arg(3).toInt();
    ledColor_B = server.arg(4).toInt();

    for (uint8_t i = start; i < end; i++) {
        strip.setPixelColor(i, strip.Color(ledColor_R, ledColor_G, ledColor_B));
    }
    strip.show();
}



void handleHallwayMotionDetected() {
    server.send(200, "text/plain", "motion");


      strip.setBrightness(230);
      strip.show();
      
    //for (int i = 5; i < 200; i += 20)
    //{
    //    strip.setBrightness(i);
    //    strip.show();
    //    delay(200);
    //    //server.handleClient();
    //}

    for (int i = 1; i < 100; i++)
    {
        server.handleClient();
        delay(900);
    }

    strip.setBrightness(5);
      strip.show();
      
    //for (int i = 200; i >= 5; i--)
    //{
      //strip.setPixelColor(i, strip.Color(ledColor_R, ledColor_G, ledColor_B));
    //    strip.setBrightness(i);
    //    strip.show();
    //    delay(100);
    //}
    

}


void handleHallwayMotionDetected_mqtt() {

      strip.setBrightness(230);
      strip.show();

   /* for (int i = 5; i < 200; i += 20)
    {
        strip.setBrightness(i);
        strip.show();
        delay(200);
    }
    */

    for (int i = 1; i < 100; i++)
    {
        delay(900);
    }

    strip.setBrightness(5);
      strip.show();
      
    //for (int i = 200; i >= 5; i--)
    //{
      //strip.setPixelColor(i, strip.Color(ledColor_R, ledColor_G, ledColor_B));
    //    strip.setBrightness(i);
    //    strip.show();
    //    delay(100);
    //}
    

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

    server.on("/enableOta", handleEnableOTA);
    server.on("/setBrightness", handleSetBrightness);
    server.on("/setColor", handleSetColor);
    server.on("/setColorRange", handleSetColorRange);
   
    server.on("/hallwayMotionDetected", handleHallwayMotionDetected);


    server.begin();
    Serial.println("HTTP server started");
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW

    //Set up LED strip
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    //strip.setBrightness(10); // Set BRIGHTNESS to about 1/5 (max = 255)
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(ledColor_R, ledColor_G, ledColor_B));
    }
    strip.show();
    strip.setBrightness(2);
    strip.show();
    Serial.println("Led setup");
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
            
            String base = "wemos/hallway/";
            //base.concat(device_name);
            base.concat(WiFi.localIP().toString());
            char topic[40];
            base.toCharArray(topic, 40);
          client.subscribe(topicHallwayMotionDetected);
           client.publish(topic, "connected", true);
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
    //Serial.print("Message arrived [");
    //Serial.print(topic);
    //Serial.print("] ");
    //for (int i = 0; i < length; i++) {
    //    Serial.print((char)payload[i]);
    //}
    //Serial.println();

    handleHallwayMotionDetected_mqtt();
    //if (strcmp(topic, topicHallwayMotionDetected))
    {

        //Serial.println(";Pump downstairs");
        
    }

}
