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

  // --- Cấu hình chân Relay là OUTPUT, mặc định OFF (LOW) ---
  pinMode(PIN_RELAY_BPLUS, OUTPUT);
  digitalWrite(PIN_RELAY_BPLUS, RELAY_OFF);

  // --- Gắn Servo ---
  servoSDcard.attach(PIN_SERVO_SDCARD);
  servoOled.attach(PIN_SERVO_OLED);
  servoFontCam.attach(PIN_SERVO_FONTCAM);

  // ServoOled: Quay ngay về 0 độ (nhanh, an toàn)
  servoOled.write(SERVO_OLED_HOME);

  // ServoFontCam: Về HOME (0°) khi khởi động. Nếu đã ở 0° thì đứng yên.
  servoFontCam.write(SERVO_FONTCAM_HOME);

  // ServoSDcard: Về HOME (0°) khi khởi động. Nếu đã ở 0° thì đứng yên.
  servoSDcard.write(SERVO_SDCARD_HOME);
  lastServoMoveTime = millis();

  // --- Khởi tạo I2C tốc độ cao cho OLED để giảm hiện tượng nháy khi quay video ---
  Wire.begin();
  Wire.setClock(OLED_I2C_CLOCK_HZ);

  // Đồng bộ tốc độ bus trong U8g2 trước khi begin
  u8g2.setBusClock(OLED_I2C_CLOCK_HZ);

  // --- Khởi tạo OLED và hiển thị "YURA" ---
  u8g2.begin();

  // Tăng tần số quét nội bộ SH1106 lên tối đa để giảm banding khi quay video.
  // Lệnh 0xD5: Set Display Clock Divide Ratio / Oscillator Frequency
  //   Byte thứ 2: bits[7:4] = 0xF (tần số oscillator tối đa = 15)
  //               bits[3:0] = 0x0 (divide ratio = 1, không chia tần số)
  // Giá trị 0xF0 -> SH1106 quét nội bộ nhanh nhất có thể, khó bị rolling
  // shutter camera bắt được đường ngang giữa các lần quét.
  u8g2.sendF("ca", 0xD5, 0xF0);

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
