#include <WiFi.h>
#include <WiFiUDP.h>

const char* ssid     = "168DST24";
const char* password = "olivia15";

int port = 8089;
byte host[] = {18, 191, 146, 167};
// bi.pancamo.com 18.191.146.167

WiFiUDP udp;

const int sensorPin = A0;  

float vIN = 0.0;
float vOUT = 0.0;
float R1 = 200000.0;
float R2 = 100000.0;

float offset = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);


// Connect to WIFI

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

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

  String line;

  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)


  int sensorValue = analogRead(sensorPin);
  Serial.printf("sensorValue = %d\n", sensorValue);     
  
  vOUT = (sensorValue / 4095.0) * 5;
  
  vIN =  sensorValue * 0.00417141049;



  //Serial.printf("vOUT = %2.2f\n", vOUT);           
  Serial.printf("vIN = %2.2f\n", vIN);               

  line = String("BattMan.voltage value=" + String(vIN));
  Serial.println(line);

  // send the packet
  Serial.println("Sending UDP packet...");
  udp.beginPacket(host, port);
  udp.print(line);
  udp.endPacket();

  //delay(1000); 
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(4000);       

    
    
    
}

