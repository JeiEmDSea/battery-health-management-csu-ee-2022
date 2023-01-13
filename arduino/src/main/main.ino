#include "Arduino.h"
#include "LiquidCrystal_I2C.h"
#include "DS3231.h"
#include "SdFat.h"
#include "SimpleRelay.h"
#include "Voltmeter.h"
#include "LM35.h"

// ? PIN DEFINITIONS
#define VOLT_SENSOR_PIN A0
#define CELL_1_TEMP_SENSOR_PIN A1
#define CELL_2_TEMP_SENSOR_PIN A2
#define CELL_3_TEMP_SENSOR_PIN A3
#define CELL_1_NEGATIVE_PIN 30
#define CELL_1_POSITIVE_PIN 31
#define CELL_2_NEGATIVE_PIN 32
#define CELL_2_POSITIVE_PIN 33
#define CELL_3_NEGATIVE_PIN 34
#define CELL_3_POSITIVE_PIN 35
#define LOAD_PIN 36

// ? I/O & CALIBRATION VALUES
unsigned int voltSensorMaxVoltage = 25;   // ? Max voltage the sensor can handle
unsigned int voltSensorLowerBound = 10;   // ? Raw sensor value when voltage is zero
unsigned int voltSensorUpperBound = 1020; // ? Raw sensor value when voltage is max
float unloadedResistance = 1.5;           // ? Internal resistance measure when no load is applied
float loadedResistance = 11.4;            // ? Actual resistance measured across the load
float badInternalResistance = 1.0;        // ? Indicates a bad internal resistance value

// ? I/O OBJECTS
LiquidCrystal_I2C lcd(0x27, 20, 4);
DS3231 rtc(SDA, SCL);
SdFat sdCard;
SdFile sdFile;
// ? RELAY CHANNEL OBJECTS
SimpleRelay cell1Negative = SimpleRelay(CELL_1_NEGATIVE_PIN, true);
SimpleRelay cell1Postive = SimpleRelay(CELL_1_POSITIVE_PIN, true);
SimpleRelay cell2Negative = SimpleRelay(CELL_2_NEGATIVE_PIN, true);
SimpleRelay cell2Postive = SimpleRelay(CELL_2_POSITIVE_PIN, true);
SimpleRelay cell3Negative = SimpleRelay(CELL_3_NEGATIVE_PIN, true);
SimpleRelay cell3Postive = SimpleRelay(CELL_3_POSITIVE_PIN, true);
SimpleRelay load = SimpleRelay(LOAD_PIN, true);
// ? SENSOR OBJECTS
Voltmeter voltSensor(VOLT_SENSOR_PIN, voltSensorMaxVoltage, voltSensorLowerBound, voltSensorUpperBound);
LM35 cell1TempSensor(CELL_1_TEMP_SENSOR_PIN);
LM35 cell2TempSensor(CELL_2_TEMP_SENSOR_PIN);
LM35 cell3TempSensor(CELL_3_TEMP_SENSOR_PIN);

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
  cellTestProcedure();
  delaySeconds(60); // ? Test the cell pack every minute
}

void initLCD()
{
  lcd.init();
  lcd.backlight();
  // ! lcd.setCursor(0, 0);
  // ! lcd.print("Battery Health");
  // ! lcd.setCursor(0, 1);
  // ! lcd.print("Monitoring System");
  // ! delaySeconds(2);
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

void cellTestProcedure()
{
  bool cell1State = testCell(1);
  delaySeconds(1);
  bool cell2State = testCell(2);
  delaySeconds(1);
  bool cell3State = testCell(3);
  delaySeconds(2);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TIME: " + String(rtc.getTimeStr()));
  lcd.setCursor(0, 1);
  lcd.print("CELL 1: " + String(cell1State ? "Good" : "Bad"));
  lcd.setCursor(0, 2);
  lcd.print("CELL 2: " + String(cell2State ? "Good" : "Bad"));
  lcd.setCursor(0, 3);
  lcd.print("CELL 3: " + String(cell3State ? "Good" : "Bad"));
}

bool testCell(int cell)
{
  connectCell(cell);
  delaySeconds(3);

  float v1 = voltSensor.getVoltage();
  float i1 = v1 / unloadedResistance;
  load.on();
  delaySeconds(2);
  float v2 = voltSensor.getVoltage();
  float i2 = v2 / loadedResistance;
  float intRes = v2 <= v1 ? (v1 - v2) / (i2 - i1) * -1 : 0;
  load.off();
  disconnectCell(cell);
  float temp = getCellTemp(cell);

  recordValues(cell, v1, intRes, temp);
  displayValues(cell, v1, intRes, temp);
  // ? Use this function inplace of above to show raw data
  // ! displayTestValues(cell, v1, v2, i2, intRes);

  return intRes > 0 ? intRes <= badInternalResistance : true;
}

void displayValues(int cell, float v1, float intRes, float temp)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CELL " + String(cell));

  lcd.setCursor(0, 1);
  lcd.print("V: " + String(v1));

  lcd.setCursor(9, 1);
  lcd.print("IR: " + String(intRes, 4));

  lcd.setCursor(0, 2);
  lcd.print("TEMP: " + String(temp) + "'C");

  lcd.setCursor(0, 3);
  lcd.print("STATE: " + String(intRes > 0 ? (intRes <= badInternalResistance ? "Good cell" : "Bad cell") : "Good cell"));
}

void displayTestValues(int cell, float v1, float v2, float i2, float intRes)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CELL " + String(cell));

  lcd.setCursor(0, 1);
  lcd.print("V1: " + String(v1));

  lcd.setCursor(9, 1);
  lcd.print("V2: " + String(v2));

  lcd.setCursor(0, 2);
  lcd.print("I2: " + String(i2));

  lcd.setCursor(9, 2);
  lcd.print("IR: " + String(intRes, 4));

  lcd.setCursor(0, 3);
  lcd.print("STATE: " + String(intRes > 0 ? (intRes <= badInternalResistance ? "Good cell" : "Bad cell") : "N/A"));
}

void recordValues(int cell, float voltage, float internalResistance, float temp)
{
  String date = rtc.getDateStr();
  String logFile = String(String(date) + ".cell" + String(cell) + ".log.txt");

  if (sdFile.open(logFile.c_str(), O_CREAT | O_WRITE | O_APPEND))
  {
    sdFile.print(rtc.getTimeStr());
    sdFile.print(",");
    sdFile.print(String(voltage));
    sdFile.print(",");
    sdFile.print(String(internalResistance));
    sdFile.print(",");
    sdFile.println(String(temp));
    sdFile.close();
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error logging data");
    delay(2000);
  }
}

void connectCell(int cell)
{
  switch (cell)
  {
  case 1:
    cell1Postive.on();
    delay(100);
    cell1Negative.on();
    break;
  case 2:
    cell2Postive.on();
    delay(100);
    cell2Negative.on();
    break;
  case 3:
    cell3Postive.on();
    delay(100);
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

float getCellTemp(int cell)
{
  switch (cell)
  {
  case 1:
    return cell1TempSensor.cel();
  case 2:
    return cell2TempSensor.cel();
  case 3:
    return cell3TempSensor.cel();
  default:
    return 0.0;
  }
}

void delaySeconds(unsigned long sec)
{
  delay(sec * 1000);
}
