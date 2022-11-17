#include <Wire.h>
#include <DS1302.h>
#include <stdlib.h>

bool isAuto = true; // mac dinh khi he thong chay o che do auto
int threshold = 60; // nguong do am kich hoat may bom

int mois; // bien luu gia tri do am dat hien tai
int isOn = false; // bien luu on/off

// rtc ds1302
const int clk = 7;
const int dat = 6;
const int rst = 5;
DS1302 rtc(rst, dat, clk);

#define relayPin 8
#define sensorPin A0

const long interval = 1000; // thoi gian update 
unsigned long previousMillis = 0;

int readSensor() {
  int value = analogRead(sensorPin);
  int wet = 600;
  int dry = 1023;
  int percent = map(value, wet, dry, 100, 0);
  if (percent > 100) {
    percent = 100;
  }
  if (percent < 0) {
    percent = 0;
  }

  return percent;
}

void pumpOn() {
  digitalWrite(relayPin, HIGH);
}
void pumpOff() {
  digitalWrite(relayPin, LOW);
}

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    mois = readSensor();
    // print ra serial
    String str = String(mois);
    Serial.println("Mois = " + str);
  }
  if (isOn) {
    pumpOn();
  }
  else {
    pumpOff();
  }
}

void receiveEvent() {
  String data = "";
  while (0 <Wire.available()) {
    char c = Wire.read();
    if (c == 255) {
      break;
    }
    data = data + c;
  }
  if (data == "on") {
    isOn = true;
  }
  else {
    isOn = false;
  }
  
 Serial.print("Mode = ");  
 Serial.println(data); 
}
void requestEvent() {
  // gui gia tri do am dat (mois) sang esp
  mois = readSensor();
  String data = String(mois);
  Wire.write(data.c_str());
}
