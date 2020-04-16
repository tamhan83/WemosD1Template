#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>





#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// SCL GPIO5
// SDA GPIO4
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2



#ifndef STASSID
#define STASSID "Doghouse_2.4"
#define STAPSK  "GuitarHero40!"
#endif

#define topicBoiler "heating/boiler"
#define topicPumpDownstairs "heating/pump_downstairs"
#define topicPumpUpstairs "heating/pump_upstairs"
#define topicImageHeater "heating/image-heater"

#define topicTempMusicRoom "temperature/music_room"

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

char temperatureMr[8];
float out = 3.12;

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


    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)

    display.display();
    delay(2000);

    // Clear the buffer.
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setRotation(3);
    display.setCursor(0, 0);
    display.println("Hi Hannes!");
    //display.ssd1306_command(0x81);
    //display.ssd1306_command(5); //max 157
    display.display();
    
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
            client.subscribe(topicTempMusicRoom);
            

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


    checkStatusLight(topic, payload, length, topicBoiler, &boilerOn);
    checkStatusLight(topic, payload, length, topicPumpDownstairs, &pumpDownstairsOn);
    checkStatusLight(topic, payload, length, topicPumpUpstairs, &pumpUpstairsOn);
    checkStatusLight(topic, payload, length, topicImageHeater, &imageHeaterOn);
    checkTemp(topic, payload, length, topicTempMusicRoom);

    printStatus();

}

void printStatus()
{
    display.clearDisplay();
    display.setCursor(0, 0);
    

    if (boilerOn)
        display.println("Heat  I");
    else
        display.println("Heat  O");

    if (pumpDownstairsOn)
        display.println("Down  I");
    else
        display.println("Down  O");

    if (pumpUpstairsOn)
        display.println("Up    I");
    else
        display.println("Up    O");

    if (imageHeaterOn)
        display.println("Image I");
    else
        display.println("Image O");

    //display.println(out);
    display.print("MR ");
    display.println(temperatureMr);

    /*display.print("Gar ");
    display.println(temperatureMr);*/
    

    display.display();
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

void checkTemp(char* topic, byte* payload, unsigned int length, char* topicToCheck) {
    if (strcmp(topic, topicToCheck) == 0)
    {

        
            Serial.println("temp received");
            Serial.println(length);
            
            strncpy(temperatureMr, (char*)payload, sizeof(length));
            

        }
    
}