#include <M5Stack.h>
#define BLYNK_PRINT Serial
// ultrasonic sensor
#define TRIGGER_PIN  26
#define ECHO_PIN     25

#include <DHT.h>
#include "WiFi.h"
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>

// hardware information
float height = 30; //height of bin in cm

char auth[] = "uIe-AQ-wbVOvX_Er8L5zgsT1JfFhLy0x";
const char* ssid = "Ppp";
const char* pass = "12312312121";

long duration, distance;

#define DHTPIN 5 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

BlynkTimer timer;
String LINE_TOKEN = "RA8mMWBE9oxrGbfAlmrdY2MON4CCkynRnj4sLepTqSe";
//collectable value
float old_store = 1000;
float old_h = 100;
float old_t = 100;

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  Blynk.virtualWrite(V2,h);
  Blynk.virtualWrite(V1,t);

  //notice when temp over 100C
  if( t >= 100){
    String data = "Alam!!, Fire";
    Line_Notify(LINE_TOKEN, data);
  }
  else if( t >= 50){
    String data = "High Temp";
    Line_Notify(LINE_TOKEN, data);
  }
  //Button setting
  if(M5.BtnA.wasPressed()){
    String data = "Current temperature is " + String(t) + " Celcius" + " Current humid is " + String(h) + " percent";
    Line_Notify(LINE_TOKEN, data);
  }

  //M5 Screen
   if(h != old_h || t != old_t){
       M5.Lcd.fillScreen(0);
   }
   M5.Lcd.println("Humid Percentage");
   M5.Lcd.println(String(h) + "%");
   M5.Lcd.println("Temperature");
   M5.Lcd.println(String(t) + " C");
   old_h = h;
   old_t = t;
}

void sendDistance()
{
    digitalWrite(TRIGGER_PIN, LOW); 
    delayMicroseconds(2); 
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);

    // measuring in CM
    distance = (duration/2) / 29.1;
    
    // showing Serial
    Serial.print(distance);
    Serial.println(" cm");

    //Calculate distance to percentage
    float store = ((height - distance)/height)*100;

    if(store >= 80){
      Line_Notify(LINE_TOKEN, "Please clear the garbages");
      delay(5000);
    }

    //Button setting
    if(M5.BtnA.wasPressed()){
      String data = "Current store percentage is " + String(store) + " percent";
      Line_Notify(LINE_TOKEN, data);
    }
    
    // Blynk V0 Labeled Value Widget
    Blynk.virtualWrite(V0,store);

    //M5 Screen
      if(store != old_store){
        M5.Lcd.fillScreen(0);
      }

      M5.Lcd.setCursor(0,40);
      M5.Lcd.println("Storage Percent");
      M5.Lcd.println(String(store));
      old_store = store;
}

void Line_Notify(String LINE_Token, String message) {
  String msg = String("message=") + message;
  WiFiClientSecure client;
  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return;
  }

  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Authorization: Bearer " + String(LINE_Token) + "\r\n";
  req += "Content-Length: " + String(msg.length()) + "\r\n";
  req += "\r\n";
  req +=  msg;

  client.print(req);
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

void setup()
{
  M5.begin();
  M5.Lcd.setTextSize(3);
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  // pin mode
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT); 
  // Blynk
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);
}
void loop()
{
  sendDistance();
  delay(1000);
  Blynk.run();
  timer.run();
  M5.update();
}
