/* =====================================================================
 * main.cpp - Logic chạy chính cho Arduino UNO R3
 * ---------------------------------------------------------------------
 * Toàn bộ macro, enum, biến toàn cục, đối tượng phần cứng và các hàm
 * xử lý đã được khai báo và định nghĩa trong file "config.h".
 * File này CHỈ chứa setup() và loop() để dễ theo dõi luồng chính.
 * ===================================================================== */

#include "config.h"

// =====================================================================
// SETUP - Khởi tạo phần cứng (chạy 1 lần khi nạp/reset)
// =====================================================================
void setup()
{
  // --- Khởi tạo Serial cổng COM ---
  Serial.begin(SERIAL_BAUD);

  // --- Cấu hình các chân Relay là OUTPUT, mặc định OFF (LOW) ---
  pinMode(PIN_RELAY_BPLUS, OUTPUT);
  pinMode(PIN_RELAY_CAMFONT_1, OUTPUT);
  pinMode(PIN_RELAY_CAMFONT_2, OUTPUT);
  pinMode(PIN_RELAY_CAMREAR_1, OUTPUT);
  pinMode(PIN_RELAY_CAMREAR_2, OUTPUT);

  digitalWrite(PIN_RELAY_BPLUS, RELAY_OFF);
  digitalWrite(PIN_RELAY_CAMFONT_1, RELAY_OFF);
  digitalWrite(PIN_RELAY_CAMFONT_2, RELAY_OFF);
  digitalWrite(PIN_RELAY_CAMREAR_1, RELAY_OFF);
  digitalWrite(PIN_RELAY_CAMREAR_2, RELAY_OFF);

  // --- Gắn Servo ---
  servoSDcard.attach(PIN_SERVO_SDCARD);
  servoOled.attach(PIN_SERVO_OLED);

  // ServoOled: Quay ngay về 0 độ (nhanh, an toàn)
  servoOled.write(SERVO_OLED_HOME);

  // ServoSDcard: Khởi động ở giả định 93 độ rồi state machine tự về 0 chậm
  servoSDcard.write(sdcCurrentAngle);
  lastServoMoveTime = millis();

  // --- Khởi tạo OLED và hiển thị "YURA" ---
  u8g2.begin();
  displayBootMessage();
}

// =====================================================================
// LOOP - Vòng lặp chính, gọi liên tục các tác vụ non-blocking
// =====================================================================
void loop()
{
  readSerialNonBlocking(); // Liên tục đọc cổng COM
  updateServoSDcard();     // Cập nhật state machine ServoSDcard
  updateServoOled();       // Cập nhật state machine ServoOled
}
