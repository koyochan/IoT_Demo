// /*********** 修正ポイント ************
// 1) #include <Arduino.h> を追加 (一部環境で必要)
// 2) forward declaration: void sendNfcData(String nfcData); を追加
// 3) WiFiはESP32標準のWiFi.hを使い、WiFi.mode(), WiFi.begin() が使える
// 4) sendNfcData() を loop() より下に置く場合、上で宣言必須
// ****************************************/

// #include <Arduino.h>
// #include <M5Core2.h>    // M5Core2用ライブラリ
// #include <WiFi.h>       // ESP32標準WiFiライブラリ
// #include <HTTPClient.h>
// #include <MFRC522.h>
// #include <SPI.h>

// //-------------------------------------------------
// // 前方宣言 (loop()より後ろで定義する関数を先に宣言する)
// //-------------------------------------------------
// void sendNfcData(String nfcData);
// void printBoth(const String &message);

// //-------------------------------------------------
// // Wi-Fi設定
// //-------------------------------------------------
// const char* ssid      = "SSID-IPhone";
// const char* password  = "kvd4m7zz1bw0";
// const char* serverUrl = "https://firebase-demo-alpha.vercel.app/api/IoT/NFC";

// //-------------------------------------------------
// // RFID RC522 のピン (M5Core2裏面の内蔵RC522の場合: CS=21, RST=22)
// //-------------------------------------------------
// #define SS_PIN  21
// #define RST_PIN 22
// MFRC522 rfid(SS_PIN, RST_PIN);

// //-------------------------------------------------
// // setup()
// //-------------------------------------------------
// void setup() {
//     // M5Core2初期化
//     M5.begin();                // （M5Unified 推奨のため警告が出るが、使用可能）
//     Serial.begin(115200);

//     // LCD初期設定
//     M5.Lcd.setTextSize(2);
//     M5.Lcd.setCursor(0, 0);
//     M5.Lcd.clear();
//     M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

//     printBoth("Booting...");

//     // SPI & RFID 初期化
//     SPI.begin();
//     rfid.PCD_Init();

//     // Wi-Fi をステーションモードで接続
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(ssid, password);

//     printBoth("WiFi Connecting...");
//     while (WiFi.status() != WL_CONNECTED) {
//         // ドットを追加して接続を待機
//         Serial.print(".");
//         M5.Lcd.print(".");
//         delay(1000);
//     }
//     printBoth("\nConnected to WiFi");
//     printBoth("Please put your NFC card...");
// }

// //-------------------------------------------------
// // loop()
// //-------------------------------------------------
// void loop() {
//     // NFCカードが新しく検出されていない or 読み取り失敗の場合
//     if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
//         // ユーザへのメッセージ：カードをかざすように表示
//         delay(500);
//         return;
//     }

//     // NFCタグのUID取得（HEX文字列）
//     String uid;
//     for (byte i = 0; i < rfid.uid.size; i++) {
//         // HEX表記で足していく (小文字になるので、必要に応じて toUpperCase() を使ってもよい)
//         uid += String(rfid.uid.uidByte[i], HEX);
//     }
//     // 取得したUIDを両方に表示
//     printBoth("NFC UID: " + uid);

//     // 取得UIDをAPIへ送信
//     sendNfcData(uid);

//     // 読み取り終了、次のスキャンに備えてタグを停止
//     rfid.PICC_HaltA();
//     rfid.PCD_StopCrypto1();

//     // 5秒待機（次の読み取りを遅らせる）
//     delay(5000);
// }

// //-------------------------------------------------
// // APIにNFCデータを送信
// //-------------------------------------------------
// void sendNfcData(String nfcData) {
//     if (WiFi.status() != WL_CONNECTED) {
//         printBoth("WiFi not connected.");
//         return;
//     }

//     HTTPClient http;
//     http.begin(serverUrl);
//     http.addHeader("Content-Type", "application/json");

//     // 送信ペイロードをJSON形式で生成
//     String jsonPayload = "{\"nfc_uid\": \"" + nfcData + "\"}";

//     // POSTを実行
//     int httpResponseCode = http.POST(jsonPayload);

//     // 結果をシリアル＆LCDに出力
//     if (httpResponseCode > 0) {
//         String response = http.getString();  // レスポンス本文
//         Serial.printf("HTTP Response code: %d\n", httpResponseCode);
//         Serial.println("Response: " + response);

//         M5.Lcd.printf("HTTP Resp code: %d\n", httpResponseCode);
//         M5.Lcd.println("Resp: " + response);
//     } else {
//         printBoth("Error on HTTP request");
//     }

//     http.end();
// }

// //-------------------------------------------------
// // LCD とシリアルモニタの両方に出力する関数
// //-------------------------------------------------
// void printBoth(const String &message) {
//     Serial.println(message);
//     M5.Lcd.println(message);
// }