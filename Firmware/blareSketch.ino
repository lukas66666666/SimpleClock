#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_SCLK 9
#define TFT_MOSI 10
#define TFT_RST 8
#define TFT_DC 4
#define TFT_CS 5
#define TFT_BL 6

#define BUZZER 7
#define SW1 0
#define SW2 1
#define SW3 2

class MyST7789 : public Adafruit_ST7789 {
public:
  MyST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst)
    : Adafruit_ST7789(cs, dc, mosi, sclk, rst) {}
  void setOffsets(uint8_t col, uint8_t row) {
    _colstart = _colstart2 = col;
    _rowstart = _rowstart2 = row;
  }
};

MyST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

int hours = 1;
int minutes = 0;
int seconds = 0;

bool alarmTriggered = false;
bool alarmRinging = false;
bool settingAlarm = false;

int alarmHour = 1;
int alarmMinutes = 1;

void updateScreen() {
  tft.fillScreen(ST77XX_BLACK);

  if(alarmRinging) {
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(6);
    tft.setCursor(19, 15);
    tft.print("WAKE UP");
  }

  else if (settingAlarm) {
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(88, 5);
    tft.print("Set Alarm");

    tft.setTextSize(6);
    tft.setCursor(25, 24);
    
    tft.print(alarmHour);

    tft.print(":");

    if(alarmMinutes < 10) {
      tft.print("0");
    }
    tft.print(alarmMinutes);
  }

  else {
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(6);
    tft.setCursor(15, 6);

    tft.print(hours);
    tft.print(":");
    if (minutes < 10) {
      tft.print("0");
    }
    tft.print(minutes);
  }
}

void updateTime() {
  static unsigned long lastSec = 0;
  unsigned long now = millis();

  if (now - lastSec >= 1000) {
    lastSec += 1000;
    seconds++;

    if (seconds >= 60) {
      seconds = 0;
      minutes++;

      alarmTriggered = false;
    }
    if (minutes >= 60) {
      minutes = 0;
      hours++;
    }
    if (hours >= 24) {
      hours = 0;
    }
  }
}

void checkAlarm() {
  if (settingAlarm || alarmRinging) {
    return;
  }

  if (hours == alarmHour && minutes == alarmMinutes && !alarmTriggered) {
    alarmRinging = true;
    alarmTriggered = true;
  }
}

void updateBuzzer() {
  static unsigned long lastToggle = 0;
  static bool buzzerOn = false;

  if (!alarmRinging) {
    digitalWrite(BUZZER, LOW);
    buzzerOn = false;
    return;
  }

  if (millis() - lastToggle >= 500) {
    lastToggle = millis();
    buzzerOn = !buzzerOn;
    digitalWrite(BUZZER, buzzerOn);
  }
}

bool lastSw1 = HIGH;
bool lastSw2 = HIGH;
bool lastSw3 = HIGH;

unsigned long timeSw1 = 0;
unsigned long timeSw2 = 0;
unsigned long timeSw3 = 0;

bool buttonPressed(int pin, bool &lastState, unsigned long &lastTime) {
  bool state = digitalRead(pin);
  bool pressed = false;

  if (state != lastState && millis() - lastTime > 50) {
    lastTime = millis();
    if (state == LOW) {
      pressed = true;
    }
  }

  lastState = state;
  return pressed;
}

void checkBtns() {
  if (buttonPressed(SW1, lastSw1, timeSw1)) {
    if (settingAlarm) {
      alarmHour++;

      if(alarmHour > 23)  {
        alarmHour = 0;
      }
    }
  }

  if (buttonPressed(SW2, lastSw2, timeSw2)) {
    if (alarmRinging) {
      alarmRinging = false;
    }
    else {
      settingAlarm = !settingAlarm;
    }
  }

  if (buttonPressed(SW3, lastSw3, timeSw3)) {
    if (settingAlarm) {
      alarmMinutes++;

      if(alarmMinutes > 59)  {
        alarmMinutes = 0;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);

  tft.init(76, 284);
  tft.setOffsets(82, 18);
  tft.invertDisplay(false);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  Serial.println("TFT Initialized!");
  tft.setCursor(0, 0);

  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  updateTime();
  checkAlarm();
  updateBuzzer();
  checkBtns();

  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate >= 250) {
    lastUpdate = millis();

    updateScreen();
  }
}