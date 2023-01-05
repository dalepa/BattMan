/*
DLP 2023
Updates:
        20230105 - UPdated offset with actual observed value         
Name: BattMan 
Board: Heltec ESP32v2 LORA+WIFI
Description:  Read voltage from 23 voltage sensor and send it to UDP inflUX on aws
              Also read form DHT11 Temp Sensor
*/

#include "Arduino.h"
#include <WiFi.h>
#include <WiFiUDP.h>
#include "heltec.h"
#include <Adafruit_Sensor.h>
#include "DHTesp.h"


//DSP11
DHTesp dht;


const char* ssid     = "168DST24";
const char* password = "olivia15";
#define LED_PIN 21
#define DHT11_PIN 27
int port = 8089;
byte host[] = {18, 191, 146, 167};
// bi.pancamo.com 18.191.146.167

WiFiUDP udp;

const int sensorPin = A0;  

float vIN = 0.0;
float vOUT = 0.0;
float R1 = 200000.0;
float R2 = 100000.0;

float factor = 0.00410915775;

// the setup function runs once when you press reset or power the board
void setup() {

//DHT11
  dht.setup(DHT11_PIN, DHTesp::DHT11);

//OLED
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  Heltec.display->flipScreenVertically();


  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);


// Connect to WIFI

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        Heltec.display->drawString(0, 0, "Wifi Connecting");
    }


    Heltec.display->drawString(0, 0, "Wifi Connected");

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");



}



// the loop function runs over and over again forever
void loop() {

  digitalWrite(LED_PIN, HIGH);  // turn the LED on (HIGH is the voltage level)


  unsigned long currentMillis = millis();
  unsigned long seconds = currentMillis / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;
  String uptime = "Uptime: " + String(days) + ":" + String(hours) + ":" + String(minutes) + ":" + String(seconds);


  String line;



  int sensorValue = analogRead(sensorPin);
  Serial.printf("sensorValue = %d\n", sensorValue);     
  
 // vOUT = (sensorValue / 4095.0) * 5;
  vIN =  sensorValue * factor;

         
  Serial.printf("vIN = %2.2f\n", vIN);               
  
//OLED
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_24);
  Heltec.display->drawString(0, 0, "Volts: " + String(vIN));

  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 26, uptime);
  
//DHT11
  float temperature = dht.getTemperature()  * 1.8 + 32 ;
  float humidity = dht.getHumidity();
  String temperatureDisplay ="Temperature: " + (String)temperature +  "°F";
  String humidityDisplay = "Humimidity: " + (String)humidity + "%";
  Heltec.display->drawString(0, 38, temperatureDisplay);
  Heltec.display->drawString(0, 49, humidityDisplay);
  Serial.printf("Temp = %2.2f\n", temperature); 

  Heltec.display->display();


  // send the packet
  Serial.println("Sending UDP packet...");

  line = String("BattMan.voltage value=" + String(vIN));
  Serial.println(line);
  udp.beginPacket(host, port);
  udp.print(line);
  udp.endPacket();

  line = String("BattMan.temperature value=" + String(temperature));
  Serial.println(line);
  udp.beginPacket(host, port);
  udp.print(line);
  udp.endPacket();  

  line = String("BattMan.humidity value=" + String(humidity));
  Serial.println(line);
  udp.beginPacket(host, port);
  udp.print(line);
  udp.endPacket();  


  //delay(100); 
  digitalWrite(LED_PIN, LOW);   // turn the LED off by making the voltage LOW
  delay(4000);       

    
    
    
}
