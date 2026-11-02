#include "Clock_Mode.h"
#include "UI_Common.h"
#include "Menu_Main.h"
#include <WiFi.h>
#include <time.h>
#include <TFT_eSPI.h>

// ---------- Konfigurasi ----------
const char* WIFI_SSID = "pupuman";
const char* WIFI_PASS = "rAZKA29010000";

#define GMT_OFFSET_SEC   (7 * 3600)   // WIB
#define DST_OFFSET_SEC   0
#define NTP_SERVER_1     "pool.ntp.org"
#define NTP_SERVER_2     "time.google.com"

// ---------- Layout (landscape 320x240) ----------
#define SCR_W        320
#define Y_HEADER_H   34
#define Y_LABEL      40
#define Y_TIME       58
#define Y_DATE       112
#define Y_BAR        146
#define Y_MET_BOX    162
#define MET_X        8
#define MET_W        180
#define MET_H        56

// Area orbit (kanan bawah)
#define ORB_X        196
#define ORB_Y        162
#define ORB_W        120
#define ORB_H        56
#define ORBIT_CX     256
#define ORBIT_CY     190
#define ORBIT_RX     52
#define ORBIT_RY     16
#define EARTH_R      11

// ---------- Warna ----------
static uint16_t C_BG, C_RED, C_BLUE, C_WHITE, C_GREY, C_CYAN, C_GREEN;

// ---------- State ----------
static bool     ntpStarted   = false;
static bool     timeValid    = false;
static bool     noTimeShown  = false;
static int      lastMin      = -1;
static int      lastSec      = -1;
static int      lastDay      = -1;
static int      lastMetSec   = -1;
static bool     lastWifi     = false;
static bool     wifiDrawn    = false;
static uint32_t enterMillis  = 0;
static uint32_t lastOrbitMs  = 0;

static int16_t starX[16], starY[16];

// ---------- Helper ----------
static void initColors() {
  C_BG    = TFT_BLACK;
  C_RED   = tft.color565(252, 61, 33);
  C_BLUE  = tft.color565(11, 61, 145);
  C_WHITE = TFT_WHITE;
  C_GREY  = tft.color565(130, 130, 145);
  C_CYAN  = tft.color565(110, 180, 255);
  C_GREEN = tft.color565(40, 160, 80);
}

static void initStars() {
  uint32_t s = 12345;
  for (int i = 0; i < 16; i++) {
    s = s * 1664525UL + 1013904223UL;
    starX[i] = ORB_X + 2 + (s >> 8) % (ORB_W - 4);
    s = s * 1664525UL + 1013904223UL;
    starY[i] = ORB_Y + 2 + (s >> 8) % (ORB_H - 4);
  }
}

static void drawMeatball(int cx, int cy, int r) {
  tft.fillCircle(cx, cy, r, C_BLUE);
  tft.drawCircle(cx, cy, r, C_WHITE);
  tft.drawPixel(cx - 8, cy - 9, C_WHITE);
  tft.drawPixel(cx + 9, cy + 8, C_WHITE);
  tft.drawPixel(cx - 4, cy + 10, C_WHITE);
  tft.drawPixel(cx + 3, cy - 11, C_WHITE);
  tft.fillTriangle(cx - r + 3, cy + 7, cx + r - 3, cy - 7, cx + r - 3, cy - 3, C_RED);
  tft.fillTriangle(cx - r + 3, cy + 7, cx - r + 5, cy + 4, cx + r - 3, cy - 3, C_RED);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(C_WHITE);
  tft.drawString("NASA", cx, cy - 4, 1);
}

static void drawWifi(bool on) {
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(on ? C_GREEN : C_RED, C_BG);
  tft.setTextPadding(70);
  tft.drawString(on ? "LINK OK" : "NO LINK", SCR_W - 6, 17, 2);
  tft.setTextPadding(0);
}

static void drawStatic() {
  tft.fillScreen(C_BG);

  // Header
  drawMeatball(22, 17, 15);
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(C_WHITE, C_BG);
  tft.drawString("MISSION CLOCK", 45, 17, 2);
  tft.fillRect(0, Y_HEADER_H, SCR_W, 2, C_RED);

  // Label
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(C_GREY, C_BG);
  tft.drawString("LOCAL TIME  //  WIB (UTC+7)", SCR_W / 2, Y_LABEL, 2);

  // Kotak MET
  tft.drawRoundRect(MET_X, Y_MET_BOX, MET_W, MET_H, 6, C_BLUE);
  tft.fillRect(MET_X, Y_MET_BOX + 2, 4, MET_H - 4, C_RED);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(C_CYAN, C_BG);
  tft.drawString("MISSION ELAPSED TIME", MET_X + MET_W / 2 + 2, Y_MET_BOX + 6, 2);

  // Bingkai orbit
  tft.drawRoundRect(ORB_X, ORB_Y, ORB_W, ORB_H, 6, C_BLUE);

  // Tombol KEMBALI (Sejajar Footer)
  tft.fillRoundRect(SCR_W / 2 - 35, 230, 70, 14, 3, C_BLUE);
  tft.drawRoundRect(SCR_W / 2 - 35, 230, 70, 14, 3, C_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(C_WHITE, C_BLUE);
  tft.drawString("KEMBALI", SCR_W / 2, 237, 1);

  // Footer
  tft.fillRect(0, 224, SCR_W, 1, C_BLUE);
  tft.setTextDatum(BL_DATUM);
  tft.setTextColor(C_GREY, C_BG);
  tft.drawString("ARTEMIS PROGRAM", 8, 238, 1);
  tft.setTextDatum(BR_DATUM);
  tft.drawString("GO FOR LAUNCH", SCR_W - 8, 238, 1);

  wifiDrawn = false;
}

static void drawTime(const struct tm &t) {
  char hm[8], ss[4];
  snprintf(hm, sizeof(hm), "%02d:%02d", t.tm_hour, t.tm_min);
  snprintf(ss, sizeof(ss), "%02d", t.tm_sec);

  int wHM = tft.textWidth("00:00", 7);
  int wSS = tft.textWidth("00", 4);
  int total = wHM + 8 + wSS;
  int x0 = (SCR_W - total) / 2;

  tft.setTextDatum(TL_DATUM);
  if (t.tm_min != lastMin) {
    tft.setTextColor(C_WHITE, C_BG);
    tft.drawString(hm, x0, Y_TIME, 7);
    lastMin = t.tm_min;
  }
  tft.setTextColor(C_RED, C_BG);
  tft.setTextPadding(wSS);
  tft.drawString(ss, x0 + wHM + 8, Y_TIME + 24, 4);
  tft.setTextPadding(0);
}

static void drawNoTime() {
  int wHM = tft.textWidth("00:00", 7);
  int x0 = (SCR_W - wHM) / 2;
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(C_GREY, C_BG);
  tft.drawString("--:--", x0, Y_TIME, 7);
  tft.setTextDatum(TC_DATUM);
  tft.setTextPadding(300);
  tft.setTextColor(C_RED, C_BG);
  tft.drawString("SYNCING NTP...", SCR_W / 2, Y_DATE, 4);
  tft.setTextPadding(0);
}

static void drawDate(const struct tm &t) {
  char buf[24];
  strftime(buf, sizeof(buf), "%a %d %b %Y", &t);
  for (char *p = buf; *p; p++) *p = toupper(*p);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextPadding(300);
  tft.drawString(buf, SCR_W / 2, Y_DATE, 4);
  tft.setTextPadding(0);
}

static void drawSecBar(int sec) {
  const int x = 10, w = 300, h = 8;
  int filled = (sec * w) / 59;
  tft.drawRect(x - 1, Y_BAR - 1, w + 2, h + 2, C_BLUE);
  tft.fillRect(x, Y_BAR, filled, h, C_RED);
  tft.fillRect(x + filled, Y_BAR, w - filled, h, C_BG);
}

static void drawMET() {
  uint32_t s = (millis() - enterMillis) / 1000;
  if ((int)s == lastMetSec) return;
  lastMetSec = (int)s;
  char buf[20];
  snprintf(buf, sizeof(buf), "T+ %02lu:%02lu:%02lu",
           (unsigned long)(s / 3600 % 100),
           (unsigned long)(s / 60 % 60),
           (unsigned long)(s % 60));
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextPadding(160);
  tft.drawString(buf, MET_X + MET_W / 2 + 2, Y_MET_BOX + 28, 4);
  tft.setTextPadding(0);
}

static void drawOrbitFrame() {
  tft.fillRect(ORB_X + 2, ORB_Y + 2, ORB_W - 4, ORB_H - 4, C_BG);

  for (int i = 0; i < 16; i++) {
    tft.drawPixel(starX[i], starY[i], (i % 3 == 0) ? C_WHITE : C_GREY);
  }

  for (int a = 0; a < 360; a += 6) {
    float r = a * DEG_TO_RAD;
    tft.drawPixel(ORBIT_CX + ORBIT_RX * cosf(r),
                  ORBIT_CY + ORBIT_RY * sinf(r), C_BLUE);
  }

  tft.fillCircle(ORBIT_CX, ORBIT_CY, EARTH_R, C_BLUE);
  tft.fillCircle(ORBIT_CX - 3, ORBIT_CY - 3, 3, C_GREEN);
  tft.fillCircle(ORBIT_CX + 4, ORBIT_CY + 3, 3, C_GREEN);
  tft.drawCircle(ORBIT_CX, ORBIT_CY, EARTH_R, C_CYAN);

  float ang = (millis() % 8000) / 8000.0f * TWO_PI;
  int sx = ORBIT_CX + ORBIT_RX * cosf(ang);
  int sy = ORBIT_CY + ORBIT_RY * sinf(ang);
  tft.fillRect(sx - 2, sy - 2, 5, 5, C_WHITE);
  tft.fillRect(sx - 5, sy - 1, 3, 3, C_RED);
  tft.fillRect(sx + 3, sy - 1, 3, 3, C_RED);
}

// ---------- API ----------
void initClockMode() {
  touchLocked = true;
  initColors();
  initStars();
  enterMillis = millis();
  lastMin = lastSec = lastDay = lastMetSec = -1;
  timeValid = false;
  noTimeShown = false;
  lastOrbitMs = 0;
  tft.setRotation(1);
  drawStatic();
  drawOrbitFrame();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  ntpStarted = false;

  touchLocked = false;
}

void loopClockMode() {
  bool w = (WiFi.status() == WL_CONNECTED);
  if (!wifiDrawn || w != lastWifi) {
    drawWifi(w);
    lastWifi = w;
    wifiDrawn = true;
  }
  if (w && !ntpStarted) {
    configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER_1, NTP_SERVER_2);
    ntpStarted = true;
  }

  struct tm t;
  if (ntpStarted && getLocalTime(&t, 0)) {
    if (!timeValid) {
      timeValid = true;
      lastMin = lastSec = lastDay = -1;
      tft.fillRect(0, Y_TIME, SCR_W, Y_BAR - Y_TIME - 4, C_BG);
    }
    if (t.tm_sec != lastSec) {
      lastSec = t.tm_sec;
      drawTime(t);
      drawSecBar(t.tm_sec);
    }
    if (t.tm_mday != lastDay) {
      lastDay = t.tm_mday;
      drawDate(t);
    }
  } else if (!timeValid && !noTimeShown) {
    drawNoTime();
    noTimeShown = true;
  }

  drawMET();

  if (millis() - lastOrbitMs >= 80) {
    lastOrbitMs = millis();
    drawOrbitFrame();
  }

  // ==========================================
  // TOUCH POLLING (Tombol KEMBALI)
  // ==========================================
  static unsigned long lastTouchCheck = 0;
  if (millis() - lastTouchCheck < 50) return;
  lastTouchCheck = millis();

  uint16_t tx, ty;
  bool touched = tft.getTouch(&tx, &ty);

  if (touched && !touchLocked) {
    tx = 319 - tx; // Inversi X

    // Tombol KEMBALI (X: SCR_W/2 - 35 sampai SCR_W/2 + 35, Y: 230 sampai 245)
    if (tx >= SCR_W / 2 - 35 && tx <= SCR_W / 2 + 35 && ty >= 230 && ty <= 245) {
      Serial.println("[CLOCK] Back to menu");
      WiFi.disconnect(true);
      touchLocked = false;
      currentState = 0;
      initMenuMain();
      return;
    }
    touchLocked = true;   
  }

  if (!touched) touchLocked = false;
}