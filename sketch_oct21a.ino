#include <TFT_eSPI.h>
#include <SPI.h>
#include <RF24.h>
#include "UI_Common.h"
#include "Menu_Main.h"
#include "Jammer_Mode.h"
#include "Deauth_Mode.h"
#include "Clock_Mode.h"

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  tft.init();
  tft.setRotation(1);       // KEMBALI KE HORIZONTAL (Landscape)
  tft.invertDisplay(false);
  tft.fillScreen(COLOR_BG);

  initDeauthTask();
  initMenuMain();
  currentState = 0;
}

void loop() {
  switch (currentState) {
    case 0: loopMenuMain(); break;
    case 1: loopJammerMode(); break;
    case 2: loopDeauthMode(); break;
    case 3: loopClockMode(); break;
  }
}