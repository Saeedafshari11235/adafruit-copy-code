/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
 //my notes 001
 //it's just a copy from adafruit for researches
 //the main job of this code is making a connection between  nodemcu and adafruit using mqtt
 //--------------------------------------------
 //my notes 002
 //add esp8266 library because nodemcu uses esp8266
#include <ESP8266WiFi.h>
 //add addafruit library.I think it's for making it ready for mqtt
#include "Adafruit_MQTT.h"
//as i explain at top
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/
//my notes 003
//adding a cons var for ssid and password
//it's because your device (nodemcu) wants' to get the info from Internet
#define WLAN_SSID       "...your SSID..."
#define WLAN_PASS       "...your password..."

/************************* Adafruit.io Setup *********************************/
//my notes 004
//this one is for highlighting the site that we wanna be connected with
#define AIO_SERVER      "io.adafruit.com"
//i don't know what is it (^_^)
#define AIO_SERVERPORT  1883// use 8883 for SSL ---->!!!!!!this is not mine!!!!!!
//user name in adafruit
#define AIO_USERNAME    "...your AIO username (see https://accounts.adafruit.com)..."
//the yellow "key" written in your dashboard (middle top)
#define AIO_KEY         "...your AIO key..."

/************ Global State (you don't need to change this!) ******************/
//my notes 005
//well it doesn't need any more comment ;)
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
//my note 006
//do whatever is written down here!   just do it!   I'm serious!
// Setup a feed called 'potValue' for publishing.
//the feed must be made in your dashboard     just do it!
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>     read this line again! it is important!!!!
Adafruit_MQTT_Publish potValue = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/potValue");
//################just do it!#################
// Setup a feed called 'ledBrightness' for subscribing to changes.
Adafruit_MQTT_Subscribe ledBrightness = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ledBrightness");
//$$$$$$$$$$$$$$$$$$!!!JUST_DO_IT!!!$$$$$$$$$$$$$$$$$$$$$$$
/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
//my note 007
//it seems like this command is for making a function (تابع)
void MQTT_connect();
//i have no comments about what is it but i have to do some researches!
// . . .
// . . .
// . . .
//I did the researche!
//its shorthand for: a type of unsigned integer of length 8 bits
//well , i didn't get why it is used for here
//anyway...
uint8_t ledPin = 12;
uint16_t potAdcValue = 0;
uint16_t ledBrightValue = 0;
//finally it begins
void setup() {
  //my notes 008
  //setting the speed of serial connection
  //but i thought it would be on 115000 or sth like that
  Serial.begin(9600);
  delay(10);
  //just for showoff
  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  //connect to the WIFI
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  //while wifi is not connected read it again
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  //write the WIFI ip
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  // i will explain it later
  // Setup MQTT subscription for ledBrightness feed.
  mqtt.subscribe(&ledBrightness);
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(200))) {
    if (subscription == &ledBrightness) {
      Serial.print(F("Got LED Brightness : "));
      ledBrightValue = atoi((char *)ledBrightness.lastread);
      Serial.println(ledBrightValue);
      analogWrite(ledPin, ledBrightValue);
    }
  }

  // Now we can publish stuff!
  uint16_t AdcValue = analogRead(A0);
  if((AdcValue > (potAdcValue + 7)) || (AdcValue < (potAdcValue - 7))){
    potAdcValue = AdcValue;
    Serial.print(F("Sending pot val "));
    Serial.print(potAdcValue);
    Serial.print("...");
    if (! potValue.publish(potAdcValue)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
  }
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
