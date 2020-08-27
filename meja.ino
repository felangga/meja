#include <Arduino.h>

#include <AccelStepper.h>
#include "SevenSegmentTM1637.h"
#include "SevenSegmentExtended.h"
#include "SevenSegmentFun.h"
#include "EEPROM.h"

#define CLK 5
#define DIO 4
#define dirPin 7
#define stepPin 6
#define motorInterfaceType 1

AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
SevenSegmentFun display(CLK, DIO);

const int LONG_PRESS_TIME = 1000;
int lastStateA, lastStateB;
int currentStateA, currentStateB;
int berapaKali = 0;
int maxLevel = 10;

long waktuPencetA = 0, waktuLepasA = 0, waktuPencetB = 0, waktuLepasB = 0;
char data[10];

long posisi = 0, saveA = 0, saveB = 0, lastPosisi = -1;
bool lepas7, lepas6;
int8_t TimeDisp[] = {0x00, 0x00, 0x00, 0x00};

void setup() {

  Serial.begin(115200);

  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);


  display.begin();            // initializes the display
  display.setBacklight(100);  // set the brightness to 100 %
  display.clear();

  stepper.setMaxSpeed(2000);


  berapaKali = EEPROM.read(3);
  maxLevel = EEPROM.read(4);

  saveA = EEPROM.read(1) * (200 * berapaKali);
  saveB = EEPROM.read(2) * (200 * berapaKali);
  posisi = EEPROM.read(5) * (200 * berapaKali);
  stepper.setCurrentPosition(posisi);


  Serial.print("Posisi : " );
  Serial.println(posisi);

  lepas7 = true;
  lepas6 = true;
}

int i = 1;

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void loop() {

  if (Serial.available() > 0) {
    String serialResponse = Serial.readStringUntil('\r\n');
    String cmd = getValue(serialResponse, '=', 0);
    int val = getValue(serialResponse, '=', 1).toInt();
    
    if (cmd == "max") {
      EEPROM.write(4, val);
      Serial.println("MAX DIGANTI");
    }
    if (cmd == "putar") {
      EEPROM.write(3, val);
      Serial.println("PUTARAN DIGANTI");
    }

  }

  if (analogRead(A7) == 0) lepas7 = true;
  if (analogRead(A6) == 0) lepas6 = true;

  currentStateA = analogRead(A5);
  currentStateB = analogRead(A4);

  if (analogRead(A5) == 1023 && lastStateA == 0) {
    waktuPencetA = millis();

  } else if (analogRead(A5) == 0 && lastStateA == 1023) {
    waktuLepasA = millis();

    long hitung = waktuLepasA - waktuPencetA;
    Serial.println(hitung);
    if (hitung > LONG_PRESS_TIME) {
      saveA = posisi;
      EEPROM.write(1, posisi / (200 * berapaKali));
      Serial.println("[LOG] Posisi A tersimpan");
      display.print("SAVE TO A");
    } else {
      posisi = saveA;
      EEPROM.write(5, posisi / (200 * berapaKali));
      stepper.moveTo(posisi);
      stepper.setSpeed(1000);
      Serial.println("[LOG] Meload posisi A");
      display.print("LOAD A");
    }
  }

  if (analogRead(A4) == 1023 && lastStateB == 0) {
    waktuPencetB = millis();

  } else if (analogRead(A4) == 0 && lastStateB == 1023) {
    waktuLepasB = millis();

    long hitung = waktuLepasB - waktuPencetB;

    if (hitung > LONG_PRESS_TIME) {
      saveB = posisi;
      EEPROM.write(2, posisi / (200 * berapaKali));
      Serial.println("[LOG] Posisi B tersimpan");
      display.print("SAVE TO B");

    } else {
      posisi = saveB;
      EEPROM.write(5, posisi / (200 * berapaKali));
      stepper.moveTo(posisi);
      stepper.setSpeed(1000);
      Serial.println("[LOG] Meload posisi B");
      display.print("LOAD B");

    }
  }

  // UP
  if (analogRead(A7) == 1023 && lepas7 && posisi / (200 * berapaKali) < maxLevel) {
    posisi += (200 * berapaKali);
    EEPROM.write(5, posisi / (200 * berapaKali));
    stepper.moveTo(posisi);
    stepper.setSpeed(1000);

    lepas7 = false;
  }

  // DOWN
  if (analogRead(A6) == 1023 && lepas6 && posisi >= 1) {
    posisi -= (200 * berapaKali);
    EEPROM.write(5, posisi / (200 * berapaKali));
    stepper.moveTo(posisi);
    stepper.setSpeed(1000);

    lepas6 = false;
  }


  sprintf(data, "%4d", posisi / (200 * berapaKali));
  display.print(data);
  lastStateA = currentStateA;
  lastStateB = currentStateB;

  stepper.runSpeedToPosition();


}
