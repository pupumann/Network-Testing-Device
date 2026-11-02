#include "Jammer_Mode.h"
#include "UI_Common.h"
#include "Menu_Main.h"
#include <SPI.h>
#include <RF24.h>

#define NRF1_CE_PIN   27
#define NRF1_CSN_PIN  21
#define NRF2_CE_PIN   26
#define NRF2_CSN_PIN  13

RF24 radio1(NRF1_CE_PIN, NRF1_CSN_PIN, 16000000);
RF24 radio2(NRF2_CE_PIN, NRF2_CSN_PIN, 16000000);

bool nrf1OK = false;
bool nrf2OK = false;
int ch  = 2;
int ch1 = 42;
unsigned int flag  = 0;
unsigned int flagv = 0;
bool jamming = false;

void drawJammerStandby() {
  tft.fillScreen(COLOR_BG);
  drawTerminalHeader("JAMMER_STANDBY", COLOR_CYAN);

  tft.setTextColor(COLOR_DIM); tft.setTextSize(1);
  tft.setCursor(15, 50); tft.print("[HARDWARE STATUS]");

  tft.setCursor(15, 70); tft.setTextColor(COLOR_TEXT); tft.print("NRF_01: ");
  if (nrf1OK) { tft.setTextColor(COLOR_GREEN); tft.print("ONLINE"); } else { tft.setTextColor(COLOR_RED); tft.print("OFFLINE"); }

  tft.setCursor(15, 85); tft.setTextColor(COLOR_TEXT); tft.print("NRF_02: ");
  if (nrf2OK) { tft.setTextColor(COLOR_GREEN); tft.print("ONLINE"); } else { tft.setTextColor(COLOR_RED); tft.print("OFFLINE"); }

  tft.drawFastVLine(160, 45, 100, COLOR_SHADOW);

  tft.setCursor(175, 50); tft.setTextColor(COLOR_DIM); tft.print("[TARGET BAND]");
  tft.setCursor(175, 70); tft.setTextColor(COLOR_YELLOW); tft.print("BAND: 2.4 GHz");
  tft.setCursor(175, 85); tft.setTextColor(COLOR_YELLOW); tft.print("MODE: CARRIER");

  drawTechButton(40, 155, 240, 50, "INITIATE", COLOR_GREEN);
  drawBackButton();
}

void drawJammerActive() {
  tft.fillScreen(COLOR_BG);
  tft.fillRect(0, 0, 320, 35, COLOR_RED);
  tft.setTextColor(COLOR_BG); tft.setTextSize(2);
  tft.setCursor(10, 8); tft.print(">> JAMMING ACTIVE <<");

  tft.setTextColor(COLOR_RED); tft.setTextSize(1);
  tft.setCursor(15, 50); tft.print("!! RF OVERLOAD IN PROGRESS !!");
  
  for(int i=0; i<320; i+=10) { tft.drawFastHLine(i, 60, 5, COLOR_RED); }

  tft.setTextColor(COLOR_TEXT); tft.setTextSize(1);
  tft.setCursor(20, 80); tft.print("NRF_01 [SWEEP]");
  tft.setCursor(20, 110); tft.setTextSize(3); tft.setTextColor(COLOR_CYAN);
  tft.print("CH "); tft.print(ch);

  tft.setTextSize(1); tft.setTextColor(COLOR_TEXT);
  tft.setCursor(180, 80); tft.print("NRF_02 [SWEEP]");
  tft.setCursor(180, 110); tft.setTextSize(3); tft.setTextColor(COLOR_YELLOW);
  tft.print("CH "); tft.print(ch1);

  tft.fillRect(20, 140, 280, 4, COLOR_RED);
  tft.fillRect(20, 148, 280, 2, COLOR_YELLOW);

  drawTechButton(40, 165, 240, 45, "ABORT", COLOR_RED);
  drawBackButton();
}

void initJammerMode() {
  jamming = false; touchLocked = false; ch = 2; ch1 = 42; flag = 0; flagv = 0;
  pinMode(NRF1_CE_PIN, OUTPUT); pinMode(NRF1_CSN_PIN, OUTPUT);
  digitalWrite(NRF1_CE_PIN, LOW); digitalWrite(NRF1_CSN_PIN, HIGH);
  pinMode(NRF2_CE_PIN, OUTPUT); pinMode(NRF2_CSN_PIN, OUTPUT);
  digitalWrite(NRF2_CE_PIN, LOW); digitalWrite(NRF2_CSN_PIN, HIGH);
  delay(100);

  if (!nrf1OK && radio1.begin(&tft.getSPIinstance()) && radio1.isChipConnected()) nrf1OK = true;
  if (!nrf1OK && radio1.begin() && radio1.isChipConnected()) nrf1OK = true;
  if (!nrf2OK && radio2.begin(&tft.getSPIinstance()) && radio2.isChipConnected()) nrf2OK = true;
  if (!nrf2OK && radio2.begin() && radio2.isChipConnected()) nrf2OK = true;

  if (!nrf1OK && !nrf2OK) {
    tft.fillScreen(COLOR_RED); tft.setTextColor(COLOR_TEXT); tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM); tft.drawString("HARDWARE FAILED", 160, 120);
    tft.setTextDatum(TL_DATUM); delay(2000); currentState = 0; initMenuMain(); return;
  }

  if (nrf1OK) { radio1.setAutoAck(false); radio1.stopListening(); radio1.setRetries(0,0); radio1.setPALevel(RF24_PA_MAX,true); radio1.setDataRate(RF24_2MBPS); radio1.setCRCLength(RF24_CRC_DISABLED); }
  if (nrf2OK) { radio2.setAutoAck(false); radio2.stopListening(); radio2.setRetries(0,0); radio2.setPALevel(RF24_PA_MAX,true); radio2.setDataRate(RF24_2MBPS); radio2.setCRCLength(RF24_CRC_DISABLED); }
  
  drawJammerStandby();
}

void loopJammerMode() {
  if (jamming) {
    if (flagv == 0) ch1 += 4; else ch1 -= 4;
    if ((ch1 > 79) && (flagv == 0)) flagv = 1; else if ((ch1 < 2) && (flagv == 1)) flagv = 0;
    if (flag == 0) ch += 2; else ch -= 2;
    if ((ch > 79) && (flag == 0)) flag = 1; else if ((ch < 2) && (flag == 1)) flag = 0;
    if (nrf1OK) radio1.setChannel(ch);
    if (nrf2OK) radio2.setChannel(ch1);
  }

  static unsigned long lastTouchCheck = 0;
  if (millis() - lastTouchCheck < 500) return;
  lastTouchCheck = millis();
  
  uint16_t x, y; 
  bool touched = tft.getTouch(&x, &y);

  if (touched && !touchLocked) {
    delay(100); 
    x = 319 - x; // Rumus Horizontal
    
    if (y >= 155 && y <= 210) { 
      touchLocked = true;
      if (!jamming) {
        ch = 2; ch1 = 42; flag = 0; flagv = 0;
        if (nrf1OK) { radio1.stopListening(); radio1.flush_tx(); radio1.setChannel(ch); radio1.startConstCarrier(RF24_PA_MAX, ch); }
        if (nrf2OK) { radio2.stopListening(); radio2.flush_tx(); radio2.setChannel(ch1); radio2.startConstCarrier(RF24_PA_MAX, ch1); }
        jamming = true; drawJammerActive();
      } else {
        if (nrf1OK) { radio1.stopConstCarrier(); radio1.flush_tx(); }
        if (nrf2OK) { radio2.stopConstCarrier(); radio2.flush_tx(); }
        jamming = false; drawJammerStandby();
      }
    }
    else if (y >= 215 && y <= 235) {
      if (jamming) { if(nrf1OK){radio1.stopConstCarrier();radio1.flush_tx();} if(nrf2OK){radio2.stopConstCarrier();radio2.flush_tx();} jamming = false; }
      touchLocked = false; currentState = 0; initMenuMain();
    } else { touchLocked = true; }
  }
  if (!touched) touchLocked = false;
}