#include "Arduino.h"
#include "LiquidCrystal_I2C.h"
#include "DS3231.h"
#include "SdFat.h"
#include "SimpleRelay.h"
#include "Voltmeter.h"
#include "LM35.h"
#include <StopWatch.h>

// ? PIN DEFINITIONS
#define VOLT_SENSOR_PIN A0
#define CELL_1_NEGATIVE_PIN 30
#define CELL_1_POSITIVE_PIN 31

// ? I/O & CALIBRATION VALUES
unsigned int voltSensorMaxVoltage = 25;   // ? Max voltage the sensor can handle
// ? I/O OBJECTS
LiquidCrystal_I2C lcd(0x27, 20, 4);
DS3231 rtc(SDA, SCL);
SdFat sdCard;
SdFile sdFile;
// ? RELAY CHANNEL OBJECTS
SimpleRelay cell1Negative = SimpleRelay(CELL_1_NEGATIVE_PIN, true);
SimpleRelay cell1Postive = SimpleRelay(CELL_1_POSITIVE_PIN, true);
// ? SENSOR OBJECTS
Voltmeter voltSensor(VOLT_SENSOR_PIN, voltSensorMaxVoltage, 10);

void setup()
{
  Serial.begin(9600);
  initLCD();
  voltSensor.initialize();

  cell1Postive.on();
  delay(100);
  cell1Negative.on();
}

void loop()
{
  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.print(voltSensor.getVoltage());

  delaySeconds(3); // ? Test the cell pack every minute
}

void initLCD()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void delaySeconds(unsigned long sec)
{
  delay(sec * 1000);
}
