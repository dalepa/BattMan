/*
  PID  0xA043      -- Product ID for BlueSolar MPPT 100/15
  FW  119     -- Firmware version of controller, v1.19
  SER#  HQXXXXXXXXX   -- Serial number
  V 13790     -- Battery voltage, mV
  I -10     -- Battery current, mA
  VPV 15950     -- Panel voltage, mV
  PPV 0     -- Panel power, W
  CS  5     -- Charge state, 0 to 9
  ERR 0     -- Error code, 0 to 119
  LOAD  ON      -- Load output state, ON/OFF
  IL  0     -- Load current, mA
  H19 0       -- Yield total, kWh
  H20 0     -- Yield today, kWh
  H21 397     -- Maximum power today, W
  H22 0       -- Yield yesterday, kWh
  H23 0     -- Maximum power yesterday, W
  HSDS  0     -- Day sequence number, 0 to 365
  Checksum  l:A0002000148   -- Message checksum
*/


#include "HardwareSerial.h"
#include <Adafruit_Sensor.h>
#include "DHTesp.h"

#include <WiFi.h>
#include <WiFiUDP.h>
#include <map>
#include <algorithm>

//DSP11
DHTesp dht;
#define DHT11_PIN 27

//Normalize Data, remove the bad data with 5 samples
std::map<String, int> data[5];
// Function prototype
int medianData(std::map<String, int> (&data)[5], String code);

int  medianData(std::map<String, int> (&data)[5], String code) {
  // Access the values of the map array
  int numbers[5];

  for (int i = 0; i < 5; i++) {
    numbers[i] = data[i][code];
  }
  std::sort(numbers, numbers + 5);
  return (numbers[2]);
}



//WIFI

const char* ssid     = "168DST24";
const char* password = "olivia15";

//UDP
int port = 8089;
byte host[] = {18, 191, 146, 167};
// bi.pancamo.com 18.191.146.167
WiFiUDP udp;

int sampleCnt = 0;

//SERIAL ON RX2
HardwareSerial serialVE(2); // VE.Direct port is connected to UART1

void setup() {
  Serial.begin(9600);
  serialVE.begin(19200); // Set the baud rate for the VE.Direct port


  //DHT11
    dht.setup(DHT11_PIN, DHTesp::DHT11);

  //WIFI SETUP
    WiFi.begin(ssid, password);

    Serial.println("Wifi Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(800);
        Serial.print(".");
    }
}


void toInflux (String line)
{

      Serial.println(line);

      udp.beginPacket(host, port);
      udp.print(line);
      udp.endPacket();
  


}

void tempToInflux()
{

      //DHT11
      float temperature = dht.getTemperature()  * 1.8 + 32 ;
      float humidity = dht.getHumidity();
      //Serial.printf("Temp = %2.2f\n", temperature);   

      String line = String("BattMan.victron.HQ2212CHJG2.battery.temperature value=" + String(temperature));
      toInflux(line);
      line = String("BattMan.victron.HQ2212CHJG2.battery.humidity value=" + String(humidity));
      toInflux(line);

}


void loop() {
  // Read a line of text from the VE.Direct port


  String inputString = serialVE.readStringUntil('\n');
  // Print the line to the serial monitor
  // Serial.println(inputString);


  int spaceIndex = inputString.indexOf('\t');

  String key = inputString.substring(0, spaceIndex);
  String svalue = inputString.substring(spaceIndex + 1);

  //Serial.print(key + "=" );
  //Serial.println(svalue);

    if (key == "Checksum")
    {  
      sampleCnt++;
      delay(1000);
    }
 
    if (sampleCnt <= 4)
    {
      data[sampleCnt][key] = svalue.toInt();
      //Serial.println("Cnt=" + String(sampleCnt) + "  key: " + key + " Value=" + svalue );
    }
    else
    {
      tempToInflux();

      int value = medianData(data, "V");      
      String line = String("BattMan.victron.HQ2212CHJG2.battery.voltage value=" + String(value/1000.0));
      toInflux(line);

      value = medianData(data, "I");
      line = String("BattMan.victron.HQ2212CHJG2.battery.current value=" + String(value/1000.0));
      toInflux(line);

      value = medianData(data, "VPV");
      line = String("BattMan.victron.HQ2212CHJG2.solar.voltage value=" + String(value/1000.0));
      toInflux(line);

      value = medianData(data, "PPV");
      line = String("BattMan.victron.HQ2212CHJG2.solar.power value=" + String(value));
      toInflux(line);

      value = medianData(data, "H20");
      line = String("BattMan.victron.HQ2212CHJG2.yieldtoday value=" + String(value));
      toInflux(line);
      
      value = medianData(data, "H21");
      line = String("BattMan.victron.HQ2212CHJG2.maxpowertoday value=" + String(value));
      toInflux(line);

      value = medianData(data, "CS");
      line = String("BattMan.victron.HQ2212CHJG2.chargestate value=" + String(value));
      toInflux(line);



      sampleCnt=0;
    }      


}

