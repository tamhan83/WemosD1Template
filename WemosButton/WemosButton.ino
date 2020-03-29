// constants won't change. They're used here to
// set pin numbers:
const int buttonPin = 2;     // the number of the pushbutton pin
const int ledPin = 13;      // the number of the LED pin
// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status
void setup() {
    Serial.begin(115200);
    Serial.println("Booting");
    // initialize the LED pin as an output:
    //pinMode(ledPin, OUTPUT);
    // initialize the pushbutton pin as an input:
    //pinMode(buttonPin, INPUT_PULLUP);
    Serial.println("hi");
}
void loop() {
    
    // read the state of the pushbutton value:
    buttonState = digitalRead(buttonPin);
    // Show the state of pushbutton on serial monitor
    Serial.println(buttonState);
    // check if the pushbutton is pressed.
    // if it is, the buttonState is HIGH:
    if (buttonState == HIGH) {
        // turn LED on:
        //digitalWrite(ledPin, HIGH);
    }
    else {
        // turn LED off:
       // digitalWrite(ledPin, LOW);
    }
    // Added the delay so that we can see the output of button
    delay(100);
}