#include "LiquidCrystal_I2C.h"
#include "DS3231.h"
#include "SdFat.h"
#include "SimpleRelay.h"

LiquidCrystal_I2C lcd(0x27, 20, 4);
DS3231 rtc(SDA, SCL);
SdFat sdCard;
SdFile sdFile;

void setup()
{
  Serial.begin(9600);
  initLCD();
  initSDcard();
  rtc.begin();
}

void loop()
{
  // ? put your main code here, to run repeatedly:
}

void initLCD()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Battery Health");
  lcd.setCursor(0, 1);
  lcd.print("Monitoring System");
  delay(2000);
  lcd.clear();
}

void initSDcard()
{
  if (sdCard.begin())
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SD Card initalized");
    delay(2000);
  }
}
