#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MFRC522_I2C.h"
#include "secrets.h"

//========== 前方宣言(Forward Declaration) ==========//
void sendNfcData(String nfcData, String deviceId);
void printBoth(const String &message);
String getDeviceId();

//========== Wi-Fi設定 ==========//
const char* serverUrl = "https://pcn.10xer.education/api/access/touch";
const char* ssid = WIFI_SSID; 
const char* password = WIFI_PASSWORD;
//========== RFID設定 (I2C接続) ==========//
MFRC522 mfrc522(0x28);

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  Serial.begin(115200);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("Booting...");

  Wire.begin();
  mfrc522.PCD_Init();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  printBoth("WiFi Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    M5.Lcd.print(".");
    delay(1000);
  }
  printBoth("\nConnected to WiFi");
  printBoth("put your card ...");
}

void loop() {
  M5.update();

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  // NFC UIDを取得
  char buf[9];
  sprintf(buf, "%02X%02X%02X%02X",
          mfrc522.uid.uidByte[0], 
          mfrc522.uid.uidByte[1],
          mfrc522.uid.uidByte[2],
          mfrc522.uid.uidByte[3]);
  String uid = String(buf);

  printBoth("NFC UID: " + uid);

  // デバイスID取得
  String deviceId = getDeviceId();
  printBoth("Device ID: " + deviceId);

  // NFC UID をAPIへ送信（デバイスIDはヘッダーに追加）
  sendNfcData(uid, deviceId);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(3000);
}

//========== ESP32のユニークなデバイスIDを取得 ==========//
String getDeviceId() {
  uint64_t chipId = ESP.getEfuseMac();
  char idBuf[13]; // 12桁の16進数 + 終端文字
  sprintf(idBuf, "%04X%08X", (uint16_t)(chipId >> 32), (uint32_t)chipId);
  return String(idBuf);
}

//========== HTTP POSTでNFCデータを送信する関数 ==========//
void sendNfcData(String nfcData, String deviceId) {
  if (WiFi.status() != WL_CONNECTED) {
    printBoth("WiFi not connected.");
    return;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("IoT-Device-ID", deviceId);  // デバイスIDをヘッダーに追加

  // JSONデータ
  String jsonPayload = "{\"uuid\": \"" + nfcData + "\"}";

  //====== リクエスト内容を Serial Monitor に表示 ======//
  Serial.println("===== HTTP Request =====");
  Serial.println("POST " + String(serverUrl));
  Serial.println("IoT-Device-ID: " + deviceId);
  Serial.println("Content-Type: application/json");
  Serial.println("Body: " + jsonPayload);
  Serial.println("========================");

  int httpResponseCode = http.POST(jsonPayload);

  //====== レスポンスを M5.LCD に表示 ======//
  M5.Lcd.fillScreen(BLACK);  // 画面クリア
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(2);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    M5.Lcd.println("Response:");
    M5.Lcd.println(response);
  } else {
    M5.Lcd.println("HTTP Error");
  }

  http.end();
}

//========== シリアルとLCDに同時に出力する関数 ==========//
void printBoth(const String &message) {
  Serial.println(message);
  M5.Lcd.println(message);
}