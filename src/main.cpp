/*********** 修正ポイント ************
1) #include <Arduino.h> を追加 (一部環境で必要)
2) forward declaration: void sendNfcData(String nfcData); を追加
3) WiFiはESP32標準のWiFi.hを使い、WiFi.mode(), WiFi.begin() が使える
4) sendNfcData() を loop() より下に置く場合、上で宣言必須
****************************************/

#include <Arduino.h>
#include <M5Core2.h>
#include <WiFi.h>        // ← ESP32用 (自動でespressif32のWiFiライブラリを参照)
#include <HTTPClient.h>  
#include <MFRC522.h>
#include <SPI.h>

// 前方宣言 (loop()より後ろで定義する関数を先に宣言する)
void sendNfcData(String nfcData);

// Wi-Fi設定
const char* ssid      = "SSID-IPhone";
const char* password  = "kvd4m7zz1bw0";
const char* serverUrl = "https://firebase-demo-alpha.vercel.app/api/IoT/NFC";

// RFID 2（RC522）のピン設定 (M5Core2裏面の内蔵RC522の場合: CS=21, RST=22 など)
#define SS_PIN  21
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
    // 初期化
    M5.begin();        // deprecated warning は無視可能 (M5Unified推奨)
    Serial.begin(115200);

    // SPI 通信 & RFID 初期化
    SPI.begin();
    rfid.PCD_Init();

    // Wi-Fi (ESP32) をステーションモードで接続
    WiFi.mode(WIFI_STA);  
    // ここで "WiFi.mode(WIFI_STA)" が使えない場合は古いWiFiライブラリを参照している証拠
    WiFi.begin(ssid, password);

    Serial.print("WiFi Connecting...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nConnected to WiFi");
}

void loop() {
    // NFCタグが新しく検出されたか？
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
        delay(500);
        return;
    }

    // NFCタグのUID取得 (HEX文字列)
    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        uid += String(rfid.uid.uidByte[i], HEX);
    }
    Serial.println("NFC UID: " + uid);

    // API にデータを送信
    sendNfcData(uid);

    // NFCタグを停止 (次の読み取りに備える)
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    delay(5000);  // 5秒待機
}

// APIにNFCデータを送信 (前方宣言が必要)
void sendNfcData(String nfcData) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected.");
        return;
    }

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"nfc_uid\": \"" + nfcData + "\"}";
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        Serial.println("Response: " + http.getString());
    } else {
        Serial.println("Error on HTTP request");
    }

    http.end();
}