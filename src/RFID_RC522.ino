#include <Arduino.h>
#include <M5Unified.h>
#include <Wire.h>
#include "MFRC522_I2C.h"

// I2Cアドレス (Unit RFIDなどは通常0x28)
MFRC522 mfrc522(0x28);

// 読み取る開始ページ (NTAG215の場合, NDEFが入っているなら 4 付近)
#define NTAG_PAGE 4

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Lcd.setTextSize(2);
  M5.Lcd.println("NTAG215 Reader (Type2)");

  Wire.begin();      // I2C 初期化
  mfrc522.PCD_Init(); // RC522 初期化

  M5.Lcd.println("Please put NTAG215 card...");
}

void loop() {
  M5.update();

  // 新しいカード検出
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(200);
    return;
  }

  M5.Lcd.println("\n--- Card Detected ---");

  // UID表示
  M5.Lcd.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) M5.Lcd.print("0");
    M5.Lcd.print(mfrc522.uid.uidByte[i], HEX);
    M5.Lcd.print(" ");
  }
  M5.Lcd.println("");

  // ▼ Type2 (NTAG21x) のREADコマンド(0x30)を送る
  byte cmdBuffer[2];
  cmdBuffer[0] = 0x30;      // READコマンド
  cmdBuffer[1] = NTAG_PAGE; // 読み取りたいページ番号(4バイト単位)

  byte readData[18];        // 受信バッファ(16バイト + CRC等)
  byte readLen = sizeof(readData);

  // 戻り値はbyte (enumの STATUS_OK=0 等)
  byte result = mfrc522.PCD_TransceiveData(
      cmdBuffer, 
      2,
      readData, 
      &readLen, 
      nullptr, 
      0, 
      false
  );

  if (result != MFRC522::STATUS_OK) {
    M5.Lcd.println("Read failed");
    // 詳細メッセージ
    M5.Lcd.println(mfrc522.GetStatusCodeName(result));
    mfrc522.PICC_HaltA();
    return;
  }

  // 読み出されたデータ(16バイト)をASCIIとして解釈
  String textData;
  for (byte i = 0; i < 16; i++) {
    textData += (char)readData[i];
  }

  M5.Lcd.println("Page4-7 Data:");
  M5.Lcd.println(textData);

  // カード停止
  mfrc522.PICC_HaltA();

  delay(3000);
}