#include <WiFi.h>
#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"
#include <AceButton.h>
using namespace ace_button;

// Wifi
const char *ssid = "FRITZ!Box 7560 RU";
const char *password = "76265768296795384496";

// IP of UI
IPAddress server(192, 168, 0, 52); // numeric IP for  Ui24  (no DNS)

// Define Rotary Encoder Pins
#define ROTARY_ENCODER_A_PIN 33
#define ROTARY_ENCODER_B_PIN 32
#define ROTARY_ENCODER_BUTTON_PIN 25
#define ROTARY_ENCODER_VCC_PIN -1 /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */

#define ROTARY_ENCODER_STEPS 4

float encoderValue = 0.3;
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);


const byte numChars = 512;
char receivedChars[numChars];
boolean newData = false;


WiFiClient client;
// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = false; // set to false for better speed measurement

// ACE BUTTON
const int BUTTON_PIN1 = 27;
const int BUTTON_PIN2 = 14;
const int BUTTON_PIN3 = 12;
const int BUTTON_PIN4 = 26; //Keine Funktion

AceButton button1(BUTTON_PIN1);
AceButton button2(BUTTON_PIN2);
AceButton button3(BUTTON_PIN3);


// Forward reference to prevent Arduino compiler becoming confused.
void handleEvent(AceButton*, uint8_t, uint8_t);

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
        encoderValue = rotaryEncoder.readEncoder();
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
    pinMode(BUTTON_PIN1, INPUT_PULLUP);
    pinMode(BUTTON_PIN2, INPUT_PULLUP);
    pinMode(BUTTON_PIN3, INPUT_PULLUP);
    

   // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress); 
  
    connectWifi();
    connectServer();

    beginMicros = micros();
}

// The event handler for both buttons.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events, for both buttons.
  Serial.print(F("handleEvent(): pin: "));
  Serial.print(button->getPin());
  Serial.print(F("; eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  // Control the LED only for the Pressed and Released events of Button 1.
  // Notice that if the MCU is rebooted while the button is pressed down, no
  // event is triggered and the LED remains off.
  switch (eventType) {
    case AceButton::kEventPressed:
      if (button->getPin() == BUTTON_PIN1) {
         client.println("SETD^i.0.mute^0") ;
         client.println("SETD^i.1.mute^0") ;
         client.println("SETD^i.2.mute^1") ;
         client.println("SETD^i.3.mute^1") ;
         client.println("SETD^i.4.mute^1") ;
         client.println("SETD^i.5.mute^1") ;
         client.println("SETD^i.6.mute^1") ;
         client.println("SETD^i.7.mute^1") ;
      }
      else if(button->getPin() == BUTTON_PIN2){
         client.println("SETD^i.0.mute^1") ;
         client.println("SETD^i.1.mute^1") ;
         client.println("SETD^i.2.mute^0") ;
         client.println("SETD^i.3.mute^0") ;
         client.println("SETD^i.4.mute^1") ;
         client.println("SETD^i.5.mute^1") ;
         client.println("SETD^i.6.mute^1") ;
         client.println("SETD^i.7.mute^1") ;
      }
       else if(button->getPin() == BUTTON_PIN3){
         client.println("SETD^i.0.mute^1") ;
         client.println("SETD^i.1.mute^1") ;
         client.println("SETD^i.2.mute^1") ;
         client.println("SETD^i.3.mute^1") ;
         client.println("SETD^i.4.mute^0") ;
         client.println("SETD^i.5.mute^0") ;
         client.println("SETD^i.6.mute^1") ;
         client.println("SETD^i.7.mute^1") ;
      }
      break;
    case AceButton::kEventReleased:
      if (button->getPin() == BUTTON_PIN1) {
        //digitalWrite(LED_PIN, LED_OFF);
      }
      break;
    case AceButton::kEventClicked:
      if (button->getPin() == BUTTON_PIN2) {
        Serial.println(F("Button 2 clicked!"));
      } else if (button->getPin() == BUTTON_PIN3) {
        Serial.println(F("Button 3 clicked!"));
      }
      break;
  }
}
/*
void recvWithEndMarker(){
  static byte ndx = 0;
  char endMarker = '^';
  char rc;

  while(client.available() > 0 ) { //&& newData == false
    rc = client.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      Serial.println(rc + "  " + ndx );
      ndx++;
      if (ndx >= numChars){
        ndx = numChars -1;
      }
     }
     else {
       //receivedChars[ndx] = '\0';
       //ndx = 0;
       newData = true;
      }
            
    
  }
  if (newData == true){
    Serial.println(receivedChars);
    newData = false;
    ndx = 0;
  }
}
*/
void readChar(){
    
  if(client.available() > 0){
  //Serial.println("Reading incoming data as chars");
  
  int len = client.available();
  byte buffer[512];
  if (len > 0) {
     if (len > 512) len = 512;
     client.read(buffer, len);
  }
  if (printWebData) {
      Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    }
  
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
      
    
    byteCount = byteCount + len;
  }
}

void readStrinG(){
      if (client.available() > 0)
  {
    //read back one line from the server
    String line = client.readStringUntil('^');
   Serial.println(line);
    
    if (line == "i.0.aux.0.pan"){
      Serial.println(line);
      Serial.println(line.length());
    }
    else {
   // Serial.print("no    ");
   // Serial.println(line);
    }
 
  }

}
void loop()
{
  
    //in loop call your custom function which will process rotary encoder values
    rotary_loop();

    readChar();
    //readStrinG();
    //recvWithEndMarker();

      // Should be called every 4-5ms or faster, for the default debouncing time
  // of ~20ms.
  button1.check();
  button2.check();
  button3.check();
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
        Serial.print("connected to Soundcraft UI at: ");
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
