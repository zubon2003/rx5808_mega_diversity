#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "EEPROM.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//PIN Assign
#define spiDataPin    26 //RX5808のCH1
#define spiClockPin   24 //RX5808のCH3

uint8_t MODULE_NUM = 8;
uint8_t SLAVE_SELECT_PIN[8] = {69, 67, 65, 63, 61, 59, 57, 55}; //ArduinoのD10ピンをRX5808のCH2に1kΩの抵抗つけて接続
uint8_t RSSI_PIN[8] = {68, 66, 64, 62, 60, 58, 56, 54}; //アナログピンでRSSIを受ける(RTC6515はRSSIを0.5-1.1Vの電圧で出力する。）

#define aMuxPinS0    28
#define aMuxPinS1    30
#define aMuxPinS2    32
#define aMuxPinS3    34

#define gndButtonPin    49
#define upButtonPin     47
#define rightButtonPin  45
#define leftButtonPin   43
#define downButtonPin   41
#define cancelButtonPin 39
#define enterButtonPin  37

uint16_t CHANNEL_FREQ_TABLE[40] = {
  5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725,
  5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866,
  5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945,
  5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880,
  5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917
};

uint16_t BAND_CHANNEL_TABLE[40] = {
  11, 12, 13, 14, 15, 16, 17, 18,
  21, 22, 23, 24, 25, 26, 27, 28,
  31, 32, 33, 34, 35, 36, 37, 38,
  41, 42, 43, 44, 45, 46, 47, 48,
  51, 52, 53, 54, 55, 56, 57, 58
};

uint16_t SORTED_CHANNEL_FREQ_TABLE[40];
uint16_t SORTED_BAND_CHANNEL_TABLE[40];

uint16_t MAX_RSSI[8] = {230, 230, 230, 230, 230, 230, 230, 230};
uint16_t MIN_RSSI[8] = {100, 100, 100, 100, 100, 100, 100, 100};

uint16_t  ACTUAL_CHANNEL = 31;
uint16_t  ACTUAL_FREQ;

void setup()
{
  sortChannel();


  // initialize with the I2C addr 0x3D or 0x3C(for the 128x64)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.clearDisplay();

  //Analog Multiplexor Control Pin
  pinMode (aMuxPinS0, OUTPUT);
  pinMode (aMuxPinS1, OUTPUT);
  pinMode (aMuxPinS2, OUTPUT);
  pinMode (aMuxPinS3, OUTPUT);

  //BUTTONS
  pinMode (gndButtonPin, OUTPUT);
  digitalWrite(gndButtonPin, LOW);
  pinMode (leftButtonPin, INPUT_PULLUP);
  pinMode (upButtonPin, INPUT_PULLUP);
  pinMode (downButtonPin, INPUT_PULLUP);
  pinMode (rightButtonPin, INPUT_PULLUP);
  pinMode (cancelButtonPin, INPUT_PULLUP);
  pinMode (enterButtonPin, INPUT_PULLUP);

  // SPI pins for RX control
  pinMode (spiDataPin, OUTPUT);
  pinMode (spiClockPin, OUTPUT);
  for (uint8_t i = 0; i < MODULE_NUM; i++) pinMode(SLAVE_SELECT_PIN[i], OUTPUT);

  digitalWrite(aMuxPinS0, LOW);
  digitalWrite(aMuxPinS1, LOW);
  digitalWrite(aMuxPinS2, LOW);
  digitalWrite(aMuxPinS3, LOW);

  

  //EEPROMより設定値読み込み
  if (readEeprom() == 0) writeEeprom();

  ACTUAL_FREQ = getFreq(ACTUAL_CHANNEL);
  setChannel();

}

void loop()
{
  int menu = 0;
  int forceRedraw = 1;
  while (1)
  {
  if (forceRedraw == 1)
    {
      initMenu();
      writeRect(menu);
      forceRedraw = 0;
    }
  if (upButtonPushed()) 
  {
    eraseRect(menu);
    if (menu == 0) menu = 2;
    else menu--;
    writeRect(menu);
  }
  if (downButtonPushed()) 
  {
    eraseRect(menu);
    if (menu == 2) menu = 0;
    else menu++;
    writeRect(menu);
  }
  if (enterButtonPushed())
  {
    switch (menu)
    {
      case 0:
        forceRedraw = 1;
        diversity();
        break;
      case 1:
        forceRedraw = 1;
        rssiCalib();
        break;
      case 2:
        forceRedraw = 1;
        saveEeprom();
        break;
    }
  }
  if (leftButtonPushed() && rightButtonPushed()) rssiCalib();
  if (upButtonPushed() && downButtonPushed()) diversity();
  }
}
void initMenu()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 4);
  display.print("DIVERSITY");
  display.setCursor(6, 26);
  display.print("RSSI CALIB");
  display.setCursor(6, 48);
  display.print("SAVEEEPROM");
  display.setTextSize(1);
  display.display();
}
void writeRect(uint8_t menu)
{
   display.drawRect(0, 22*menu+1,128,19, WHITE);
   display.display();
}
void eraseRect(uint8_t menu)
{
   display.drawRect(0, 22*menu+1,128,19, BLACK);
   display.display();
}

void diversity()
{
  uint8_t cnt = 0;
  uint8_t tmpRssi = 0;
  uint8_t maxModule = 0;
  uint16_t maxRssi = 0;
  uint32_t now;
  uint32_t nextTime = 0;
  uint16_t mRssi[8][10];
  uint16_t sumRssi[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  for (uint8_t i = 0; i < MODULE_NUM; i++)
  {
    for (uint8_t j = 0; j < 10; j++)
    {
      mRssi[i][j] = 0;
    }
  }

  initDisplay();
  printFreq();
  
  while (1)
  {
    if (upButtonPushed()) nextFreq();
    if (downButtonPushed()) prevFreq();
    if (leftButtonPushed()) nextBand();
    if (rightButtonPushed()) nextChannel();
    display.fillRect(20, 0, 40, 63, BLACK);
    now = millis();
    if (nextTime < now | nextTime == 0)
    {
      cnt++;
      if (cnt > 9) cnt = 0;

      maxRssi = 0;
      maxModule = 0;
      for (uint8_t i = 0; i < MODULE_NUM; i++)
      {
        //sumRssi[i] = sumRssi[i] - mRssi[i][cnt];
        //mRssi[i][cnt] = mappedRSSI(i);
        //sumRssi[i] = sumRssi[i] + mRssi[i][cnt];
        
        mRssi[i][cnt] = mappedRSSI(i);
        sumRssi[i] = 0;
        for (uint8_t j = 0; j < 10; j++) sumRssi[i] = sumRssi[i] + mRssi[i][j];
        if (sumRssi[i] >= maxRssi)
        {
          maxRssi = sumRssi[i];
          maxModule = i;
        }
        setCursorPrint(sumRssi[i], i);
      }
      changeAvout(maxModule);
      display.display();
      if (cancelButtonPushed()) break;
      nextTime = millis() + 100;
    }
  }
}
void changeAvout(uint8_t maxModule)
{
  avOut(maxModule);
  display.setTextSize(1);
  display.setCursor(20, 8 * maxModule);
  display.print("*");
}
void printFreq()
{ 
  display.fillRect(64, 0, 127, 63, BLACK);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(64, 0);
  display.println(ACTUAL_FREQ);
  display.setCursor(68, 32);
  display.setTextSize(4);
  display.println(ACTUAL_CHANNEL);
  display.setTextSize(1);
}

void setCursorPrint(uint16_t rssi, uint8_t module)
{
  if (rssi < 10) display.setCursor(30 + 12, 8 * module);
  else if (rssi < 100) display.setCursor(30 + 6, 8 * module);
  else display.setCursor(30, 8 * module);
  display.print(rssi);
}

void rssiCalib()
{
  uint16_t tempRSSI;
  uint32_t now;
  uint32_t nextTime = 0;

  for (uint8_t i = 0; i < MODULE_NUM; i++)
  {
    MAX_RSSI[i] = 130;
    MIN_RSSI[i] = 130;
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("TURN OFF ALL VTX AND PUSH UP BUTTON");
  display.display();
  while (1)
  {
    if (upButtonPushed()) break;
    if (cancelButtonPushed()) return;
  }
  initDisplay();

  now = millis();
  nextTime = now + 5000;
  while (nextTime > now )
  {
    display.fillRect(20, 0, 40, 63, BLACK);
    for (uint8_t i = 0; i < MODULE_NUM; i++)
    {
      display.setCursor(30, 8 * i);
      tempRSSI = readRSSI(i);
      if (MIN_RSSI[i] > tempRSSI)  MIN_RSSI[i] = tempRSSI;
      display.print(MIN_RSSI[i]);
    }
    display.setCursor(55, 0);
    display.setTextSize(3);

    display.print(ACTUAL_FREQ);
    display.setTextSize(1);
    display.display();
    now = millis();
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("TURN ON VTX AND PUSH UP BUTTON");
  display.display();
  while (1)
  {
    if (upButtonPushed()) break;
    if (cancelButtonPushed())
    {
     readEeprom();
     return;
    }
  }
  initDisplay();

  now = millis();
  nextTime = now + 5000;
  while (nextTime > now )
  {
    display.fillRect(20, 0, 40, 63, BLACK);
    for (uint8_t i = 0; i < MODULE_NUM; i++)
    {
      display.setCursor(30, 8 * i);
      tempRSSI = readRSSI(i);
      if (MAX_RSSI[i] < tempRSSI)  MAX_RSSI[i] = tempRSSI;
      display.print(MAX_RSSI[i]);
    }
    display.setCursor(55, 0);
    display.setTextSize(3);
    display.print(ACTUAL_FREQ);
    display.setTextSize(1);
    display.display();
    now = millis();
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("CALIBRATION FINISHED");
  delay(2000);
}

void saveEeprom()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("UP BUTTON=>WRITE EEPROM DOWN RELOAD EEPROM");
  display.display();
  while (1)
  {
    if (upButtonPushed())
    {
      writeEeprom();
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println("WRITE EEPROM FINISHED");
      delay(2000);
      break;
    }
    if (downButtonPushed())
    {
      readEeprom();
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println("RELOAD EEPROM FINISHED");
      delay(2000);
      break;
    }
  }
}
void initDisplay()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("M0");
  display.setCursor(0, 8 * 1);
  display.print("M1");
  display.setCursor(0, 8 * 2);
  display.print("M2");
  display.setCursor(0, 8 * 3);
  display.print("M3");
  display.setCursor(0, 8 * 4);
  display.print("M4");
  display.setCursor(0, 8 * 5);
  display.print("M5");
  display.setCursor(0, 8 * 6);
  display.print("M6");
  display.setCursor(0, 8 * 7);
  display.print("M7");
  display.display();

}

uint16_t mappedRSSI(uint8_t module)
{
  uint16_t rawRssi = 0;
  uint16_t mapRssi = 0;
  rawRssi = readRSSI(module);
  rawRssi = constrain(rawRssi, MIN_RSSI[module], MAX_RSSI[module]);
  mapRssi = map(rawRssi, MIN_RSSI[module], MAX_RSSI[module], 0, 100);
  return (mapRssi);
}

//現在のRSSIを取得するサブルーチン
//5V arduinoでは、0-5vの間を 0-1023の値に変換
//RTC6715のRSSIは 0.5v-1.1vの間で出力
//なので、生のrssi値は102-225位になる（大体）
uint16_t readRSSI(uint8_t module)
{
  uint16_t rssi = 0;
  for (uint8_t i = 0; i < 10; i++)
  {
    rssi += analogRead(RSSI_PIN[module]);
  }
  rssi = rssi / 10; // 10回測定する平均値
  return (rssi);
}

void avOut(uint8_t moduleID)
{
  switch (moduleID)
  {
    //Module0 AVOutput->Multiplexor C3
    case 0:
      digitalWrite(aMuxPinS0, HIGH);
      digitalWrite(aMuxPinS1, HIGH);
      digitalWrite(aMuxPinS2, LOW);
      digitalWrite(aMuxPinS3, LOW);
      break;
    //Module1 AVOutput->Multiplexor C2
    case 1:
      digitalWrite(aMuxPinS0, LOW);
      digitalWrite(aMuxPinS1, HIGH);
      digitalWrite(aMuxPinS2, LOW);
      digitalWrite(aMuxPinS3, LOW);
      break;
    //Module2 AVOutput->Multiplexor C1
    case 2:
      digitalWrite(aMuxPinS0, HIGH);
      digitalWrite(aMuxPinS1, LOW);
      digitalWrite(aMuxPinS2, LOW);
      digitalWrite(aMuxPinS3, LOW);
      break;
    //Module3 AVOutput->Multiplexor C0
    case 3:
      digitalWrite(aMuxPinS0, LOW);
      digitalWrite(aMuxPinS1, LOW);
      digitalWrite(aMuxPinS2, LOW);
      digitalWrite(aMuxPinS3, LOW);
      break;
    //Module4 AVOutput->Multiplexor C7
    case 4:
      digitalWrite(aMuxPinS0, HIGH);
      digitalWrite(aMuxPinS1, HIGH);
      digitalWrite(aMuxPinS2, HIGH);
      digitalWrite(aMuxPinS3, LOW);
      break;
    //Module5 AVOutput->Multiplexor C6
    case 5:
      digitalWrite(aMuxPinS0, LOW);
      digitalWrite(aMuxPinS1, HIGH);
      digitalWrite(aMuxPinS2, HIGH);
      digitalWrite(aMuxPinS3, LOW);
      break;
    //Module6 AVOutput->Multiplexor C5
    case 6:
      digitalWrite(aMuxPinS0, HIGH);
      digitalWrite(aMuxPinS1, LOW);
      digitalWrite(aMuxPinS2, HIGH);
      digitalWrite(aMuxPinS3, LOW);
      break;
    //Module7 AVOutput->Multiplexor C4
    case 7:
      digitalWrite(aMuxPinS0, LOW);
      digitalWrite(aMuxPinS1, LOW);
      digitalWrite(aMuxPinS2, HIGH);
      digitalWrite(aMuxPinS3, LOW);
      break;
  }
}

