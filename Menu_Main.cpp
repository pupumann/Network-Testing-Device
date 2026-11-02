#include "Menu_Main.h"
#include "UI_Common.h"
#include "Jammer_Mode.h"
#include "Deauth_Mode.h" 
#include "Clock_Mode.h"

void drawMenuItems() {
  tft.fillScreen(COLOR_BG);
  drawTerminalHeader("DASHBOARD", COLOR_CYAN);

  const char* items[] = { "NRF JAMMER", "DEAUTH WiFi", "CLOCK / MET" };
  uint16_t colors[] = {COLOR_RED, COLOR_GREEN, COLOR_CYAN};

  tft.setTextSize(1); tft.setTextColor(COLOR_DIM);
  tft.setCursor(15, 45); tft.print("SELECT MODULE TO EXECUTE:");

  for (int i = 0; i < 3; i++) {
    int y = 65 + (i * 45); 
    tft.drawRect(15, y, 290, 35, COLOR_SHADOW);
    tft.fillRect(15, y, 5, 35, colors[i]); 
    
    tft.setTextColor(colors[i]); tft.setTextSize(2);
    tft.setCursor(30, y + 8); tft.print("0"); tft.print(i+1); tft.print(".");
    tft.setCursor(80, y + 8); tft.print(items[i]);
    
    tft.setTextColor(COLOR_DIM); tft.setTextSize(2);
    tft.setCursor(280, y + 8); tft.print(">");
  }
  
  tft.setTextColor(COLOR_DIM); tft.setTextSize(1);
  tft.setCursor(15, 225); tft.print("TOUCH TO INITIALIZE // V.10.0");
}

void initMenuMain() {
  touchLocked = false;
  drawMenuItems();
}

bool loopMenuMain() {
  static unsigned long lastTouchCheck = 0;
  if (millis() - lastTouchCheck < 500) return false;
  lastTouchCheck = millis();

  uint16_t x, y;
  bool touched = tft.getTouch(&x, &y);

  if (touched && !touchLocked) {
    delay(100);
    x = 319 - x; // Rumus Horizontal
    
    for (int i = 0; i < 3; i++) {
      int menuY = 65 + (i * 45);
      if (y >= menuY && y <= menuY + 35) {
        touchLocked = true;
        switch (i) {
          case 0: currentState = 1; initJammerMode(); return true;
          case 1: currentState = 2; initDeauthMode(); break;
          case 2: currentState = 3; initClockMode(); break;
        }
        return true;
      }
    }
    touchLocked = true;
  }
  if (!touched) touchLocked = false;
  return false;
}