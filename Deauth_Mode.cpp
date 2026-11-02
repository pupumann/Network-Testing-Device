#include "Deauth_Mode.h"
#include "UI_Common.h"
#include "Menu_Main.h"

int deauthUIState = 0; 
int scrollOffset = 0;

void drawDeauthMenuUI() {
  tft.fillScreen(COLOR_BG);
  drawTerminalHeader("DEAUTH_MODULE", COLOR_GREEN);

  tft.setTextColor(COLOR_DIM); tft.setTextSize(1);
  tft.setCursor(15, 50); tft.print("WiFi Deauthentication Protocol");

  drawTechButton(10, 80, 300, 40, "SCAN ACCESS POINTS", COLOR_CYAN);
  drawTechButton(10, 130, 300, 40, "ATTACK ALL (BROADCAST)", COLOR_RED);
  drawTechButton(10, 180, 300, 40, "SELECT TARGET", COLOR_YELLOW);
  drawBackButton();
}

void drawScanningUI() {
  tft.fillScreen(COLOR_BG);
  drawTerminalHeader("SCANNING...", COLOR_YELLOW);
  tft.setTextColor(COLOR_YELLOW); tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("PLEASE WAIT...", 160, 100);
  tft.setTextDatum(TL_DATUM);
}

void drawListUI() {
  tft.fillScreen(COLOR_BG);
  drawTerminalHeader("AP_LIST_FOUND", COLOR_CYAN);

  if (targetCount == 0) {
    tft.setTextColor(COLOR_RED); tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM); tft.drawString("NO TARGET FOUND", 160, 100);
    tft.setTextDatum(TL_DATUM); drawBackButton(); return;
  }

  int maxVisible = 3;
  for (int i = 0; i < maxVisible; i++) {
    int idx = i + scrollOffset;
    if (idx >= targetCount) break;
    int y = 45 + (i * 40);

    if (idx == selectedTargetIndex) {
      tft.fillRect(10, y, 300, 35, COLOR_YELLOW);
      tft.setTextColor(COLOR_BG);
    } else {
      tft.drawRect(10, y, 300, 35, COLOR_SHADOW);
      tft.fillRect(10, y, 4, 35, COLOR_CYAN);
      tft.setTextColor(COLOR_TEXT);
    }

    tft.setTextSize(1);
    tft.setCursor(20, y + 5); tft.print(targets[idx].ssid.substring(0, 22));
    tft.setCursor(20, y + 20); tft.print("CH:"); tft.print(targets[idx].channel); tft.print(" | RSSI:"); tft.print(targets[idx].rssi); tft.print("dBm");
  }

  drawTechButton(10, 175, 90, 30, "UP", COLOR_CYAN);
  drawTechButton(115, 175, 90, 30, "DN", COLOR_CYAN);
  drawTechButton(220, 175, 90, 30, "SEL", COLOR_YELLOW);
  drawBackButton();
}

void drawAttackingUI() {
  tft.fillScreen(COLOR_BG);
  tft.fillRect(0, 0, 320, 35, COLOR_RED);
  tft.setTextColor(COLOR_BG); tft.setTextSize(2);
  tft.setCursor(10, 8); tft.print(">> DEAUTH ACTIVE <<");

  tft.setTextColor(COLOR_TEXT); tft.setTextSize(1);
  tft.setCursor(15, 50); tft.print("TARGET LOCKED:");
  
  tft.setTextColor(COLOR_YELLOW); tft.setTextSize(2);
  tft.setCursor(15, 65);
  if (selectedTargetIndex >= 0 && selectedTargetIndex < targetCount) {
    tft.print(targets[selectedTargetIndex].ssid.substring(0, 18));
  } else { tft.print("ALL BROADCAST"); }

  tft.setTextColor(COLOR_DIM); tft.setTextSize(1);
  tft.setCursor(15, 100); tft.print("PACKETS TRANSMITTED:");

  tft.setTextColor(COLOR_GREEN); tft.setTextSize(3);
  tft.setCursor(15, 115); tft.print(packetCount);

  drawTechButton(10, 175, 300, 40, "STOP ATTACK", COLOR_RED);
  drawBackButton();
}

void initDeauthMode() {
  deauthUIState = 0; touchLocked = false;
  selectedTargetIndex = -1; scrollOffset = 0;
  drawDeauthMenuUI();
}

void loopDeauthMode() {
  if (deauthUIState == 3 && isAttacking) {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 300) {
      tft.fillRect(15, 115, 290, 35, COLOR_BG);
      tft.setTextColor(COLOR_GREEN); tft.setTextSize(3);
      tft.setCursor(15, 115); tft.print(packetCount);
      lastUpdate = millis();
    }
  }

  static unsigned long lastTouchCheck = 0;
  if (millis() - lastTouchCheck < 500) return;
  lastTouchCheck = millis();

  uint16_t x, y;
  bool touched = tft.getTouch(&x, &y);

  if (touched && !touchLocked) {
    delay(100); 
    x = 319 - x; // Rumus Horizontal

    if (deauthUIState == 0) {
      if (y >= 80 && y <= 120) { 
        touchLocked = true; drawScanningUI(); delay(200); performWiFiScan();
        deauthUIState = 2; scrollOffset = 0; drawListUI();
      }
      else if (y >= 130 && y <= 170) { 
        touchLocked = true;
        if (targetCount > 0) { selectedTargetIndex = -1; packetCount = 0; startJamming(); deauthUIState = 3; drawAttackingUI(); }
      }
      else if (y >= 180 && y <= 220) { 
        touchLocked = true;
        if (targetCount > 0) { deauthUIState = 2; scrollOffset = 0; drawListUI(); }
      }
      else if (y >= 215 && y <= 235) { touchLocked = false; currentState = 0; initMenuMain(); }
      else { touchLocked = true; }
    }
    else if (deauthUIState == 2) {
      for (int i = 0; i < 3; i++) {
        int yItem = 45 + (i * 40);
        if (y >= yItem && y <= yItem + 35) {
          int idx = i + scrollOffset;
          if (idx < targetCount) { selectedTargetIndex = idx; touchLocked = true; drawListUI(); return; }
        }
      }
      if (y >= 175 && y <= 205) {
        if (x >= 10 && x <= 100) { if (scrollOffset > 0) scrollOffset--; touchLocked = true; drawListUI(); }
        else if (x >= 115 && x <= 205) { if (scrollOffset + 3 < targetCount) scrollOffset++; touchLocked = true; drawListUI(); }
        else if (x >= 220 && x <= 310) { 
          if (selectedTargetIndex != -1) { packetCount = 0; startJamming(); deauthUIState = 3; drawAttackingUI(); }
        }
      }
      else if (y >= 215 && y <= 235) { touchLocked = false; deauthUIState = 0; drawDeauthMenuUI(); }
      else { touchLocked = true; }
    }
    else if (deauthUIState == 3) {
      if (y >= 175 && y <= 215) { stopJamming(); touchLocked = false; deauthUIState = 0; drawDeauthMenuUI(); }
      else if (y >= 215 && y <= 235) { stopJamming(); touchLocked = false; deauthUIState = 0; drawDeauthMenuUI(); }
      else { touchLocked = true; }
    }
  }
  if (!touched) touchLocked = false;
}