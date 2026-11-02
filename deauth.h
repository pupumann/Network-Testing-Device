#ifndef DEAUTH_H
#define DEAUTH_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

extern int targetCount;
extern int selectedTargetIndex; 
extern bool isAttacking;
extern unsigned long packetCount;

struct TargetInfo {
  uint8_t bssid[6];
  int channel;
  int rssi;
  String ssid;
};
extern TargetInfo targets[20];

void performWiFiScan();
void startJamming();
void stopJamming();
void initDeauthTask();

#endif