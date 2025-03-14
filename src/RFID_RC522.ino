#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MFRC522_I2C.h"

//========== 前方宣言(Forward Declaration) ==========//
void sendNfcData(String nfcData);
void printBoth(const String &message);

//========== Wi-Fi設定 ==========//
const char* ssid      = "SSID-IPhone";
const char* password  = "kvd4m7zz1bw0";
const char* serverUrl = "https://firebase-demo-alpha.vercel.app/api/IoT/NFC";

//========== RFID設定 (I2C接続) ==========//
// I2Cアドレス (Unit RFIDの場合、通常は 0x28)
MFRC522 mfrc522(0x28);

void setup() {
  // ----- M5 Unified初期化 -----
  auto cfg = M5.config();
  M5.begin(cfg);

  // シリアルモニタ開始
  Serial.begin(115200);

  // 画面表示設定
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("Booting...");

  // ----- I2C・RFID初期化 -----
  Wire.begin();
  mfrc522.PCD_Init();   // RC522 初期化

  // ----- Wi-Fi(ESP32)接続 -----
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  printBoth("WiFi Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    M5.Lcd.print(".");
    delay(1000);
  }
  printBoth("\nConnected to WiFi");
}

void loop() {
  M5.update();  // M5Unifiedでボタンなどの状態を更新

  // 新しいカードがあるか確認
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  // ----- UID取得 -----
  char buf[9];  // UIDを格納するバッファ（8桁の16進数 + 終端）
  sprintf(buf, "%02X%02X%02X%02X",
          mfrc522.uid.uidByte[0], 
          mfrc522.uid.uidByte[1],
          mfrc522.uid.uidByte[2],
          mfrc522.uid.uidByte[3]);
  String uid = String(buf);

  printBoth("NFC UID: " + uid);

  // ----- 取得UIDをAPIへ送信 -----
  sendNfcData(uid);

  // ----- 次の読み取りに備えカードを停止 -----
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(3000);  // 読み取り間隔の調整
}

//========== HTTP POSTでNFCデータを送信する関数 ==========//
void sendNfcData(String nfcData) {
  if (WiFi.status() != WL_CONNECTED) {
    printBoth("WiFi not connected.");
    return;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  // JSON形式でUIDを送信
  String jsonPayload = "{\"nfc_uid\": \"" + nfcData + "\"}";
  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    String response = http.getString();
    printBoth("HTTP Response: " + String(httpResponseCode));
    printBoth("Response: " + response);
  } else {
    printBoth("Error on HTTP request");
  }

  http.end();
}

//========== シリアルとLCDに同時に出力する関数 ==========//
void printBoth(const String &message) {
  Serial.println(message);
  M5.Lcd.println(message);
}