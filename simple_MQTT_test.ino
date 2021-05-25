
/*
    Simple MQTT connection test using ESP8266
    created by
    Garrett Schmidt
    Senior Product Manager- Communication Interfaces
    Phoenix Contact
    +1 717 702 3226
    grschmidt@phoenixcontact.com

   May 2021
*/
//NOTE:  SSL is not included for simplicity.  
//DO NOT USE THIS TO TRANSMIT SENSITIVE DATA.

#include <ESP8266WiFi.h> //ESP8266 Wifi library
#include <ArduinoMqttClient.h> //MQTT library

#include <DHT.h> //DHT sensor library

/*********************Configure these settings**********************/
const char* Wifi_ssid     = "";                  //Wifi network SSID
const char* Wifi_password = "";             //Wifi password
const char broker[] = "";     //MQTT broker address
int        port     = ;                  //MQTT broker TCP port number, default is 1883 for unsecure
const char* mqtt_user = "";                 //MQTT user name
const char* mqtt_password = "";             //MQTT password 
const char* topic1 = "";           //MQTT publish topic name 1, for Adafruit IO, the full topic is "mqtt_user/feeds/topic1"
const char* topic2 = "";       //MQTT publish topic name 2, for Adafruit IO, the full topic is "mqtt_user/feeds/topic2"
const char* topic3 = "";            //MQTT subscribe topic name, for Adafruit IO, the full topic is "mqtt_user/feeds/topic3"
int samplerate1 = 5000;                        //how often do we take a measurement and publish it (in milliseconds)
int samplerate2 = 100;                         //how often we check topic3 that we're subscribed to
/*********************************************************************/

#define DHTPIN 4 //The GPIO pin that the DHT sensor is connected to
#define DHTTYPE DHT11 //Select the sensor type:  DHT11 or DHT22
unsigned long previousPub = 0; 
unsigned long previousSub = 0; 
//WiFiClientSecure wifiClient;
//static const char *fingerprint PROGMEM = "59 3C 48 0A B1 8B 39 4E 0D 58 50 47 9A 13 55 60 CC A0 1D AF";

WiFiClient wifiClient;//create a Wifi connection instance

MqttClient mqttClient(wifiClient); //create an MQTT instance

DHT dht(DHTPIN, DHTTYPE); //create a DHT sensor instance

//TimedAction publish_topic = TimedAction(samplerate1, pub);  //Create a timed action to call our publish function at the interval defined by samplerate1
//TimedAction subscribe_topic = TimedAction(samplerate2, sub); //Create a timed action to call our subscribe function at the interval defined by samplerate2

void setup() {//setup function, runs once on start up
  Serial.begin(115200); //start the Serial port at 115200bps for local monitoring/troubleshooting
  delay(100); //wait 100ms
  dht.begin(); //start the DHT instance
  mqttClient.setUsernamePassword(mqtt_user, mqtt_password);  //set the user name and password for the MQTT broker

  /*********We start by connecting to a WiFi network********************/
  Serial.println(); //print the connection status to the serial monitor
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(Wifi_ssid);

  WiFi.begin(Wifi_ssid, Wifi_password);  //begin the Wifi connection to the specified network

  while (WiFi.status() != WL_CONNECTED) { //while Wifi is not connected, print a "." to the serial monitor
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); //print the IP address of this device
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());  //print the subnet mask of this device
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());  //print the gateway IP address of this device
  
 // wifiClient.setFingerprint(fingerprint);
  /*****************************************************************/

  /***************Connect to the MQTT broker**********************************/
  Serial.print("Attempting to connect to the MQTT broker: "); //print the connection status to the serial monitor
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {  //if the MQTT connection fails, print the error code to the serial monitor
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
  mqttClient.subscribe(topic3);//subscribe to a topic

  /*****************************************************************/
  pinMode(2, OUTPUT); //Set GPIO pin 2 to output, which controls the Blue LED on the Huzzah
  digitalWrite(2, HIGH);  //Pin 2 is pulled high, so set the output high to turn the LED off
}//setup

void loop() {  //the main loop, it runs continuously
  unsigned long currentPub = millis();
    if (currentPub - previousPub >= samplerate1) {//check our publish function and see if it's time to run it
    previousPub = currentPub;
    pub();
    }

  delay(100); //delay 100ms

  unsigned long currentSub = millis();
    if (currentSub - previousSub >= samplerate2) {//check our subscribe function and see if it's time to run it
    previousSub = currentSub;
    sub();
    }
  
}//loop

void pub() {//the publish topics function
  mqttClient.poll(); // we call poll() regularly to allow the library to send MQTT keep alives which avoids being disconnected by the broker

  float h = dht.readHumidity(); //read the temperature, assign it to a variable h
  Serial.println(h); //print humidity to the serial monitor

  float f = dht.readTemperature(true);  // Read temperature as Fahrenheit (isFahrenheit = true), assign it to a variable t
  Serial.println(f); // print temperature to the serial monitor

  mqttClient.beginMessage(topic1);  //begin the MQTT message to topic1
  mqttClient.print(f); //enter the temperature measurement into the MQTT message
  mqttClient.endMessage(); //finish the message
  mqttClient.beginMessage(topic2); //begin the MQTT message to topic2
  mqttClient.print(h); //enter the humidity measurement into the MQTT message
  mqttClient.endMessage(); //finish the message
}//pub

void sub() { //the subscribe to topics function
  int messageSize = mqttClient.parseMessage();
  String read_MQTT; //create a string variable
  if (messageSize) {  // we received a message, print out the topic and contents
    while (mqttClient.available()) {
      read_MQTT = (char)mqttClient.read(); //read the contents of the message and save them in the read_MQTT variable
      Serial.print(read_MQTT); //print the message to the serial monitor
      Serial.println();
      if (read_MQTT == "1") { //if the message was "1", turn on the Blue LED
        digitalWrite(2, LOW);
      }
      if (read_MQTT == "0") { //if the message was "0", turn off the Blue LED
        digitalWrite(2, HIGH);
      }
    }
  }
}//sub
