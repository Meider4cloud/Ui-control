#include <WiFi.h>
#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"

// Wifi
const char *ssid = "FRITZ!Box 7560 RU";
const char *password = "76265768296795384496";

// IP of UI
IPAddress server(192, 168, 0, 52); // numeric IP for  Ui24  (no DNS)

// Define Rotary Encoder Pins
#define ROTARY_ENCODER_A_PIN 32
#define ROTARY_ENCODER_B_PIN 33
#define ROTARY_ENCODER_BUTTON_PIN 25
#define ROTARY_ENCODER_VCC_PIN -1 /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */

#define ROTARY_ENCODER_STEPS 4

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

//Add some pins to test (making sure they are not used with the shield)
int inPin = 9;  // pushbutton connected to digital pin 7
int val = 0;    // variable to store the read value
int oldval = 0; //variable to store old val

WiFiClient client;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true; // set to false for better speed measurement

void rotary_onButtonClick()
{
    static unsigned long lastTimePressed = 0;
    //ignore multiple press in that time milliseconds
    if (millis() - lastTimePressed < 500)
    {
        return;
    }
    lastTimePressed = millis();
    Serial.print("button pressed ");
    Serial.print(millis());
    Serial.println(" milliseconds after restart");
}

void rotary_loop()
{
    //dont print anything unless value changed
    if (rotaryEncoder.encoderChanged())
    {
        Serial.print("Value: ");
        Serial.println(rotaryEncoder.readEncoder());
        float encoderValue = rotaryEncoder.readEncoder();
        encoderValue = encoderValue/100;
        Serial.println(encoderValue);
        client.print("SETD^m.mix^");

        client.println(encoderValue);
    }
    if (rotaryEncoder.isEncoderButtonClicked())
    {
        rotary_onButtonClick();
    }
}

void IRAM_ATTR readEncoderISR()
{
    rotaryEncoder.readEncoder_ISR();
}

void setup()
{

    Serial.begin(115200);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    //we must initialize rotary encoder
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    //set boundaries and if values should cycle or not
    //in this example we will set possible values between 0 and 1000;
    bool circleValues = false;
    rotaryEncoder.setBoundaries(0, 100, circleValues); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

    /*Rotary acceleration introduced 25.2.2021.
   * in case range to select is huge, for example - select a value between 0 and 1000 and we want 785
   * without accelerateion you need long time to get to that number
   * Using acceleration, faster you turn, faster will the value raise.
   * For fine tuning slow down.
   */
    //rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
    rotaryEncoder.setAcceleration(100); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration

    // Setup the pin to read
    pinMode(inPin, INPUT_PULLUP); // sets the digital pin 7 as input

    connectWifi();
    connectServer();

    beginMicros = micros();
}

void loop()
{
  
    //in loop call your custom function which will process rotary encoder values
    rotary_loop();

       
    String bufferArray[512];
    int i = 0;
    
    
    if (client.available() > 0)
  {
    //read back one line from the server
    String line = client.readStringUntil('\r');
    Serial.print(i);
    Serial.print(": ");
    Serial.println(line);

    i++;
 
  }
 //Serial.println(bufferArray[200]);           
 
    
  String wantedVal = "SETD^m.mix^0";
  

    // if the server's disconnected, stop the client:
    if (!client.connected())
    {
        endMicros = micros();
        Serial.println();
        Serial.println("disconnecting.");
        client.stop();

        // do nothing forevermore:
        while (true)
        {
            delay(1);
        }
    }

    
    //read the pins we need here
    // val = digitalRead(inPin); // read the input pin
    if (oldval != val)
    { //check if the value is the same, we don't want multiple sends
        Serial.println(val);
        oldval = val; // Make the old value same as the new value
        if (val == 1)
            client.println("SETD^i.0.mute^1"); // Send SETD Mute with NEW LINE
        if (val == 0)
            client.println("SETD^i.0.mute^0");
    }
}


void connectWifi()
{
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
void connectServer()
{

    // if you get a connection, report back via serial:
    if (client.connect(server, 80))
    {
        Serial.print("connected to ");
        Serial.println(client.remoteIP());
        // Make a HTTP request:

        client.println("GET /raw HTTP1.1\n\n");

        client.println("Connection: close");
        client.println();
    }
    else
    {
        // if you didn't get a connection to the server:
        Serial.println("connection failed");
    }
}
