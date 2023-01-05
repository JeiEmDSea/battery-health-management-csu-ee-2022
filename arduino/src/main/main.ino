#include "Arduino.h"
#include "LiquidCrystal_I2C.h"
#include "DS3231.h"
#include "SdFat.h"
#include "SimpleRelay.h"
#include "Voltmeter.h"
#include "PreciseLM35.h"

// ? PIN DEFINITIONS
#define VOLT_SENSOR_PIN A0
#define CELL_1_TEMP_SENSOR_PIN A2
#define CELL_2_TEMP_SENSOR_PIN A3
#define CELL_3_TEMP_SENSOR_PIN A4

// ? I/O & CALIBRATION VALUES
unsigned int voltSensorMaxVoltage = 25;
unsigned int voltSensorLowerBound = 50;
unsigned int voltSensorUpperBound = 1023;

// ? OBJECTS
LiquidCrystal_I2C lcd(0x27, 20, 4);
DS3231 rtc(SDA, SCL);
SdFat sdCard;
SdFile sdFile;
// ? relay channels
SimpleRelay cell1Negative = SimpleRelay(2, true);
SimpleRelay cell1Postive = SimpleRelay(3, true);
SimpleRelay cell2Negative = SimpleRelay(4, true);
SimpleRelay cell2Postive = SimpleRelay(5, true);
SimpleRelay cell3Negative = SimpleRelay(6, true);
SimpleRelay cell3Postive = SimpleRelay(7, true);
SimpleRelay load = SimpleRelay(8, true);
// ? sensors
Voltmeter voltSensor(VOLT_SENSOR_PIN, voltSensorMaxVoltage, voltSensorLowerBound, voltSensorUpperBound);
PreciseLM35 cell1TempSensor(CELL_1_TEMP_SENSOR_PIN);
PreciseLM35 cell2TempSensor(CELL_2_TEMP_SENSOR_PIN);
PreciseLM35 cell3TempSensor(CELL_3_TEMP_SENSOR_PIN);

void setup()
{
  Serial.begin(9600);
  initLCD();
  initSDcard();
  voltSensor.initialize();
  rtc.begin();
  turnOffRelayChannels();
}

void loop()
{
    // ? put your main code here, to run repeatedly
  delaySeconds(1);
}

void initLCD()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Battery Health");
  lcd.setCursor(0, 1);
  lcd.print("Monitoring System");
  delaySeconds(2);
  lcd.clear();
}

void initSDcard()
{
  if (sdCard.begin())
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SD Card initalized");
    delaySeconds(2);
  }
}

void turnOffRelayChannels()
{
  load.off();
  cell1Negative.off();
  cell1Postive.off();
  cell2Negative.off();
  cell2Postive.off();
  cell3Negative.off();
  cell3Postive.off();
}

void testCell(int cell)
{
  connectCell(cell);
  // ! record values
  load.on();
  delay(150);
  // ! record values
  delay(150);
  load.off();
  disconnectCell(cell);
}

void connectCell(int cell)
{
  switch (cell)
  {
  case 1:
    cell1Postive.on();
    cell1Negative.on();
    break;
  case 2:
    cell2Postive.on();
    cell2Negative.on();
    break;
  case 3:
    cell3Postive.on();
    cell3Negative.on();
    break;
  default:
    break;
  }
}

void disconnectCell(int cell)
{
  switch (cell)
  {
  case 1:
    cell1Postive.off();
    cell1Negative.off();
    break;
  case 2:
    cell2Postive.off();
    cell2Negative.off();
    break;
  case 3:
    cell3Postive.off();
    cell3Negative.off();
    break;
  default:
    break;
  }
}

void delaySeconds(unsigned long sec)
{
  delay(sec * 1000);
}
