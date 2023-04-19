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
unsigned int voltSensorMaxVoltage = 25; // ? Max voltage the sensor can handle
float unloadedResistance = 1.0;         // ? Internal resistance measure when no load is applied
float loadedResistance = 10.9;          // ? Actual resistance measured across the load
float badInternalResistance = 0.5;      // ? Indicates a bad internal resistance value
float mTreshold = 0.02;                 // ? slope treshold

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
Voltmeter voltSensor(VOLT_SENSOR_PIN, voltSensorMaxVoltage, 10);
LM35 cell1TempSensor(CELL_1_TEMP_SENSOR_PIN);
LM35 cell2TempSensor(CELL_2_TEMP_SENSOR_PIN);
LM35 cell3TempSensor(CELL_3_TEMP_SENSOR_PIN);
// ? TIMER OBJECTS
StopWatch cell1Timer(StopWatch::MINUTES);
StopWatch cell2Timer(StopWatch::MINUTES);
StopWatch cell3Timer(StopWatch::MINUTES);
// ? INITIAL VOLTAGE
float cell1InitialVoltage = 0;
float cell2InitialVoltage = 0;
float cell3InitialVoltage = 0;
// ? CELL VOLTAGE READINGS
float cell1VoltageReading = 0;
float cell2VoltageReading = 0;
float cell3VoltageReading = 0;

void setup()
{
  Serial.begin(9600);
  initLCD();
  initSDcard();
  voltSensor.initialize();
  rtc.begin();
  turnOffRelayChannels();

  cell1InitialVoltage = getInitialVoltage(1);
  cell2InitialVoltage = getInitialVoltage(2);
  cell3InitialVoltage = getInitialVoltage(3);
}

void loop()
{
  cellTestProcedure();
  delaySeconds(10); // ? Test the cell pack every minute
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
  lcd.print("CELL 1: " + String(cell1State ? "Good" : "Bad"));
  lcd.setCursor(0, 1);
  lcd.print("CELL 2: " + String(cell2State ? "Good" : "Bad"));
  lcd.setCursor(0, 2);
  lcd.print("CELL 3: " + String(cell3State ? "Good" : "Bad"));

  delaySeconds(3);
  lcd.setCursor(0, 3);
  lcd.print("ALL: " + String(cell1VoltageReading + cell2VoltageReading + cell3VoltageReading) + "V");
}

bool testCell(int cell)
{
  connectCell(cell);
  delaySeconds(3);

  Serial.print("V1-");
  Serial.print(cell);
  Serial.print(": ");
  Serial.println(analogRead(VOLT_SENSOR_PIN));
  float v1 = voltSensor.getVoltage();

  if (v1 <= 3.0)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Place cell " + String(cell));
    while (v1 <= 3.0)
    {
      v1 = voltSensor.getVoltage();
      delaySeconds(2);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cell " + String(cell) + " detected!");

    switch (cell)
    {
    case 1:
      if (cell1Timer.isRunning())
        cell1Timer.reset();
      cell2InitialVoltage = v1;
      break;
    case 2:
      if (cell2Timer.isRunning())
        cell2Timer.reset();
      cell2InitialVoltage = v1;
      break;
    case 3:
      if (cell3Timer.isRunning())
        cell3Timer.reset();
      cell3InitialVoltage = v1;
      break;
    default:
      break;
    }
  }

  switch (cell)
  {
  case 1:
    cell1VoltageReading = v1;
    break;
  case 2:
    cell2VoltageReading = v1;
    break;
  case 3:
    cell3VoltageReading = v1;
    break;
  default:
    break;
  }

  Serial.print("V1-");
  Serial.print(cell);
  Serial.print(": ");
  Serial.println(v1);

  float i1 = v1 / unloadedResistance;
  Serial.print("I1: ");
  Serial.println(i1);

  load.on();
  delaySeconds(2);

  Serial.print("V2-");
  Serial.print(cell);
  Serial.print(": ");
  Serial.println(analogRead(VOLT_SENSOR_PIN));
  float v2 = voltSensor.getVoltage();
  Serial.print("V2-");
  Serial.print(cell);
  Serial.print(": ");
  Serial.println(v2);

  float i2 = v2 / loadedResistance;
  Serial.print("I2: ");
  Serial.println(i2);

  float intRes = v2 > v1 ? (v1 - v2) / (i2 - i1) : 0;
  Serial.print("IR: ");
  Serial.println(intRes);
  Serial.println();

  load.off();
  disconnectCell(cell);
  float temp = getCellTemp(cell);

  bool isGood = checkChargeRate(cell, v1);

  recordValues(cell, v1, intRes, temp);
  displayValues(cell, v1, intRes, temp, isGood);
  // ? Use this function inplace of above to show raw data
  // ! displayTestValues(cell, v1, v2, i2, intRes);

  return intRes > 0 ? intRes <= badInternalResistance : true;
}

bool checkChargeRate(int cell, float voltage)
{
  bool result = true;

  switch (cell)
  {
  case 1:
    if (!cell1Timer.isRunning() && voltage > cell1InitialVoltage)
      cell1Timer.start();
    break;
  case 2:
    if (!cell2Timer.isRunning() && voltage > cell2InitialVoltage)
      cell2Timer.start();
    break;
  case 3:
    if (!cell3Timer.isRunning() && voltage > cell3InitialVoltage)
      cell3Timer.start();
    break;
  default:
    break;
  }

  if (voltage >= 4.2)
  {
    switch (cell)
    {
    case 1:
      if (cell1Timer.isRunning() && cell1Timer.elapsed() > 0)
      {
        result = calculateM(cell1InitialVoltage, cell1Timer.elapsed()) < mTreshold;
        cell1Timer.stop();
      }
      break;
    case 2:
      if (cell2Timer.isRunning() && cell2Timer.elapsed() > 0)
      {
        result = calculateM(cell2InitialVoltage, cell2Timer.elapsed()) < mTreshold;
        cell2Timer.stop();
      }
      break;
    case 3:
      if (cell3Timer.isRunning() && cell3Timer.elapsed() > 0)
      {
        result = calculateM(cell3InitialVoltage, cell3Timer.elapsed()) < mTreshold;
        cell3Timer.stop();
      }
      break;
    default:
      break;
    }
  }

  return result;
}

float calculateM(float initalVoltage, int elapsed)
{
  float m = (4.2 - initalVoltage) / (elapsed - 0);
  return m;
}

float getInitialVoltage(int cell)
{
  connectCell(cell);
  delaySeconds(3);
  float result = voltSensor.getVoltage();
  disconnectCell(cell);
  return result;
}

void displayValues(int cell, float v1, float intRes, float temp, bool isGood)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CELL " + String(cell));
  // ! lcd.setCursor(9, 0);
  // ! lcd.print(String(analogRead(VOLT_SENSOR_PIN)));

  lcd.setCursor(0, 1);
  lcd.print("V: " + String(v1));

  lcd.setCursor(9, 1);
  lcd.print("IR: " + String(intRes, 3));

  lcd.setCursor(0, 2);
  lcd.print("TEMP: " + String(temp) + "'C");

  lcd.setCursor(0, 3);
  lcd.print("STATE: " + String(isGood ? "Good cell" : "Bad cell"));
}

void displayTestValues(int cell, float v1, float v2, float i2, float intRes, bool isGood)
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
  lcd.print("IR: " + String(intRes, 3));

  lcd.setCursor(0, 3);
  lcd.print("STATE: " + String(isGood ? "Good cell" : "Bad cell"));
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
