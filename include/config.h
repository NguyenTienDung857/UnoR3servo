/* =====================================================================
 * config.h - File header cấu hình + định nghĩa hàm cho dự án Arduino UNO R3
 * ---------------------------------------------------------------------
 * BẢNG GIÁ TRỊ NHẬN QUA CỔNG COM (kết thúc bằng ký tự '\n')
 * ---------------------------------------------------------------------
 *  Chuỗi 1 ký tự (Relay & Servo):
 *    '1' -> Bật RelayBplus      (D2  = HIGH)
 *    '2' -> Tắt RelayBplus      (D2  = LOW)
 *    '3' -> Bật RelayCamFont    (D7, D8 = HIGH)
 *    '4' -> Tắt RelayCamFont    (D7, D8 = LOW)
 *    '5' -> Bật RelayCamRear    (D10, D12 = HIGH)
 *    '6' -> Tắt RelayCamRear    (D10, D12 = LOW)
 *    '7' -> Chạy 1 chu kỳ ServoSDcard (D5):
 *           HOME(0) -> 93 (rất chậm ~3s) -> chờ 1s -> HOME(0) (rất chậm ~3s)
 *    '8' -> Chạy 1 chu kỳ ServoOled  (D3):
 *           HOME(0) -> 90 (nhanh) -> chờ 0.5s -> HOME(0) (nhanh)
 *
 *  Chuỗi 2 ký tự (OLED):
 *    "44" -> Xóa toàn bộ màn hình OLED (clear screen)
 *
 *  Chuỗi 5 ký tự (OLED):
 *    "ABCDE" -> Hiển thị 5 chữ số lên OLED:
 *      P1 = ký tự thứ 1, P2 = ký tự 2-3, P3 = ký tự 4-5
 *      Dòng 1: P1 sát mép trái, P2 sát mép phải (chữ to)
 *      Dòng 2: P3 căn giữa (chữ to)
 *      Số 0 ở đầu của P2 hoặc P3 sẽ bị bỏ (vd "06" -> "6")
 *      Ví dụ: "10203" -> Dòng 1 "1   2", Dòng 2 "3"
 *      Ví dụ: "21003" -> Dòng 1 "2  10", Dòng 2 "3"
 *      Ví dụ: "30112" -> Dòng 1 "3   1", Dòng 2 "12"
 *
 *  CHÚ Ý: Toàn bộ code dùng millis() (Non-blocking), KHÔNG dùng delay().
 *
 *  CẤU TRÚC FILE:
 *    - Phần 1: Macro (RELAY, PIN, SERVO, SERIAL)
 *    - Phần 2: Enum trạng thái State Machine
 *    - Phần 3: Đối tượng OLED, Servo
 *    - Phần 4: Biến toàn cục
 *    - Phần 5: Định nghĩa các hàm xử lý
 * ===================================================================== */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Servo.h>
#include <U8g2lib.h>
#include <Wire.h>

/* =====================================================================
 * PHẦN 1: MACRO
 * ===================================================================== */

// ---------------------------------------------------------------------
// MACRO TRẠNG THÁI RELAY
// Nếu module relay là loại Active LOW thì đảo lại 2 macro này.
// ---------------------------------------------------------------------
#define RELAY_ON   HIGH    // Mức logic để BẬT relay
#define RELAY_OFF  LOW     // Mức logic để TẮT relay

// ---------------------------------------------------------------------
// CHÂN CẮM (PIN) PHẦN CỨNG
// ---------------------------------------------------------------------
#define PIN_RELAY_BPLUS       2     // D2  -> RelayBplus
#define PIN_RELAY_CAMFONT_1   7     // D7  -> RelayCamFont_1
#define PIN_RELAY_CAMFONT_2   8     // D8  -> RelayCamFont_2
#define PIN_RELAY_CAMREAR_1   10    // D10 -> RelayCamRear_1
#define PIN_RELAY_CAMREAR_2   12    // D12 -> RelayCamRear_2
#define PIN_SERVO_SDCARD      5     // D5  -> ServoSDcard
#define PIN_SERVO_OLED        3     // D3  -> ServoOled

// ---------------------------------------------------------------------
// THÔNG SỐ ĐIỀU KHIỂN SERVO
// ---------------------------------------------------------------------
#define SERVO_SDCARD_HOME       0      // Vị trí gốc HOME của ServoSDcard
#define SERVO_SDCARD_MAX        93     // Góc tối đa (KHÔNG ĐƯỢC quá 93 độ)
#define SERVO_SDCARD_STEP_MS    32     // Thời gian giữa 2 bước 1 độ (ms)
                                       // 93 độ * 32ms ~= 2976ms (~3 giây)
#define SERVO_SDCARD_WAIT_MS    1000   // Thời gian chờ tại 93 độ (1 giây)

#define SERVO_OLED_HOME         0      // Vị trí gốc HOME của ServoOled
#define SERVO_OLED_MAX          90     // Góc đích của ServoOled
#define SERVO_OLED_WAIT_MS      500    // Thời gian chờ tại 90 độ (0.5 giây)

// ---------------------------------------------------------------------
// THÔNG SỐ GIAO TIẾP SERIAL
// ---------------------------------------------------------------------
#define SERIAL_BAUD             9600   // Tốc độ baud cổng COM
#define SERIAL_BUFFER_SIZE      16     // Kích thước buffer chuỗi nhận

/* =====================================================================
 * PHẦN 2: ENUM TRẠNG THÁI STATE MACHINE
 * ===================================================================== */

// ---------------------------------------------------------------------
// ENUM TRẠNG THÁI CHO ServoSDcard
// IDLE           : Đứng yên tại vị trí hiện tại (chờ lệnh)
// MOVING_TO_93   : Đang quay từ từ tiến lên góc 93 độ
// WAITING_1S     : Đã đạt 93 độ, đang chờ 1 giây
// MOVING_TO_HOME : Đang quay từ từ lùi về 0 độ
// ---------------------------------------------------------------------
enum ServoSDcardState {
  SDC_IDLE = 0,
  SDC_MOVING_TO_93,
  SDC_WAITING_1S,
  SDC_MOVING_TO_HOME
};

// ---------------------------------------------------------------------
// ENUM TRẠNG THÁI CHO ServoOled
// IDLE  : Đứng yên (chờ lệnh)
// AT_90 : Đã quay tới 90 độ, đang chờ 0.5 giây
// ---------------------------------------------------------------------
enum ServoOledState {
  OLED_SV_IDLE = 0,
  OLED_SV_AT_90
};

/* =====================================================================
 * PHẦN 3: ĐỐI TƯỢNG OLED & SERVO
 * ===================================================================== */

// ---------------------------------------------------------------------
// OLED SH1106 128x64 - I2C - Phần cứng (HW I2C)
// U8G2_R2 = xoay 180 độ (vì màn hình lắp ngược)
// "_1_" = page buffer (~128 byte) -> tiết kiệm RAM cho UNO R3 (2KB SRAM)
// ---------------------------------------------------------------------
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

Servo servoSDcard;   // Servo D5 - quay chậm, max 93 độ
Servo servoOled;     // Servo D3 - quay nhanh, max 90 độ

/* =====================================================================
 * PHẦN 4: BIẾN TOÀN CỤC
 * ===================================================================== */

// ---------------------------------------------------------------------
// Biến trạng thái ServoSDcard (Non-blocking State Machine)
// ---------------------------------------------------------------------
ServoSDcardState sdcState = SDC_MOVING_TO_HOME;  // Khởi động: tự về home chậm
int  sdcCurrentAngle      = SERVO_SDCARD_MAX;    // Giả định khởi động từ 93°
unsigned long lastServoMoveTime = 0;             // Mốc thời gian bước cuối
unsigned long sdcWaitStartTime  = 0;             // Mốc bắt đầu chờ 1 giây

// ---------------------------------------------------------------------
// Biến trạng thái ServoOled (Non-blocking State Machine)
// ---------------------------------------------------------------------
ServoOledState oledSvState   = OLED_SV_IDLE;
unsigned long  oledSvWaitStart = 0;              // Mốc bắt đầu chờ 0.5s

// ---------------------------------------------------------------------
// Buffer đọc Serial (Non-blocking)
// Đọc từng ký tự nối vào buffer cho tới khi gặp '\n'
// ---------------------------------------------------------------------
char    serialBuffer[SERIAL_BUFFER_SIZE];        // Chuỗi đang nhận
uint8_t serialIndex = 0;                         // Vị trí ghi tiếp theo

// ---------------------------------------------------------------------
// Buffer hiển thị OLED (vì page buffer cần render lại nhiều lần)
// displayMode: 0 = trống, 1 = "YURA", 2 = 5 chữ số
// ---------------------------------------------------------------------
uint8_t displayMode  = 1;                        // Mặc định khởi động: "YURA"
char    displayP1[2] = {0};                      // Phần 1 (1 chữ số)
char    displayP2[3] = {0};                      // Phần 2 (đã loại số 0 đầu)
char    displayP3[3] = {0};                      // Phần 3 (đã loại số 0 đầu)

/* =====================================================================
 * PHẦN 5: ĐỊNH NGHĨA CÁC HÀM
 * ===================================================================== */

// =====================================================================
// HÀM: renderOled
// ---------------------------------------------------------------------
// Vẽ nội dung lên OLED theo trạng thái displayMode hiện tại.
// Vì U8g2 chế độ page buffer cần lặp qua từng "page", nên ta phải bọc
// toàn bộ lệnh vẽ trong vòng do { ... } while (u8g2.nextPage()).
//   displayMode = 0 -> để trống
//   displayMode = 1 -> hiển thị "YURA" to ở giữa
//   displayMode = 2 -> hiển thị 5 chữ số (theo quy tắc 3 phần)
// =====================================================================
inline void renderOled() {
  u8g2.firstPage();
  do {
    if (displayMode == 1) {
      // Hiển thị chữ "YURA" to ở giữa màn hình
      u8g2.setFont(u8g2_font_logisoso28_tr);   // Font cao 28px (có chữ cái)
      const char* msg = "YURA";
      int w = u8g2.getStrWidth(msg);
      int x = (128 - w) / 2;                   // Căn giữa ngang
      int y = 32 + 14;                         // Căn giữa dọc (baseline)
      u8g2.drawStr(x, y, msg);

    } else if (displayMode == 2) {
      // Hiển thị 5 chữ số: P1 trái, P2 phải (dòng 1), P3 giữa (dòng 2)
      u8g2.setFont(u8g2_font_logisoso28_tn);   // Font số cao 28px

      // Dòng 1: baseline y = 28
      int y1 = 28;
      u8g2.drawStr(0, y1, displayP1);                     // P1 sát trái
      int wP2 = u8g2.getStrWidth(displayP2);
      u8g2.drawStr(128 - wP2, y1, displayP2);             // P2 sát phải

      // Dòng 2: baseline y = 63 (sát đáy)
      int y2 = 63;
      int wP3 = u8g2.getStrWidth(displayP3);
      int xP3 = (128 - wP3) / 2;                          // Căn giữa
      u8g2.drawStr(xP3, y2, displayP3);
    }
    // displayMode = 0 -> không vẽ gì -> màn hình trống
  } while (u8g2.nextPage());
}

// =====================================================================
// HÀM: displayBootMessage - Hiển thị "YURA" to ở giữa khi khởi động.
// =====================================================================
inline void displayBootMessage() {
  displayMode = 1;
  renderOled();
}

// =====================================================================
// HÀM: displayClearScreen - Xóa toàn bộ màn hình OLED (lệnh "44").
// =====================================================================
inline void displayClearScreen() {
  displayMode = 0;
  renderOled();
}

// =====================================================================
// HÀM: handleFiveDigitDisplay
// ---------------------------------------------------------------------
// Tách chuỗi 5 chữ số thành 3 phần và lưu vào buffer hiển thị, sau đó
// vẽ lại OLED:
//   - P1 = digits[0]            -> luôn in
//   - P2 = digits[1..2]         -> bỏ số 0 đầu (vd "06" -> "6")
//   - P3 = digits[3..4]         -> bỏ số 0 đầu (vd "04" -> "4")
// =====================================================================
inline void handleFiveDigitDisplay(const char* digits) {
  // P1: 1 ký tự
  displayP1[0] = digits[0];
  displayP1[1] = '\0';

  // P2: bỏ số 0 đầu nếu có
  if (digits[1] == '0') {
    displayP2[0] = digits[2];
    displayP2[1] = '\0';
  } else {
    displayP2[0] = digits[1];
    displayP2[1] = digits[2];
    displayP2[2] = '\0';
  }

  // P3: bỏ số 0 đầu nếu có
  if (digits[3] == '0') {
    displayP3[0] = digits[4];
    displayP3[1] = '\0';
  } else {
    displayP3[0] = digits[3];
    displayP3[1] = digits[4];
    displayP3[2] = '\0';
  }

  displayMode = 2;
  renderOled();
}

// =====================================================================
// HÀM: handleOneCharCommand - Xử lý lệnh 1 ký tự cho Relay và Servo.
// =====================================================================
inline void handleOneCharCommand(char c) {
  switch (c) {
    case '1':  // Bật RelayBplus
      digitalWrite(PIN_RELAY_BPLUS, RELAY_ON);
      break;
    case '2':  // Tắt RelayBplus
      digitalWrite(PIN_RELAY_BPLUS, RELAY_OFF);
      break;

    case '3':  // Bật cặp RelayCamFont
      digitalWrite(PIN_RELAY_CAMFONT_1, RELAY_ON);
      digitalWrite(PIN_RELAY_CAMFONT_2, RELAY_ON);
      break;
    case '4':  // Tắt cặp RelayCamFont
      digitalWrite(PIN_RELAY_CAMFONT_1, RELAY_OFF);
      digitalWrite(PIN_RELAY_CAMFONT_2, RELAY_OFF);
      break;

    case '5':  // Bật cặp RelayCamRear
      digitalWrite(PIN_RELAY_CAMREAR_1, RELAY_ON);
      digitalWrite(PIN_RELAY_CAMREAR_2, RELAY_ON);
      break;
    case '6':  // Tắt cặp RelayCamRear
      digitalWrite(PIN_RELAY_CAMREAR_1, RELAY_OFF);
      digitalWrite(PIN_RELAY_CAMREAR_2, RELAY_OFF);
      break;

    case '7':  // Khởi động chu kỳ ServoSDcard
      // Chỉ cho phép chu kỳ mới khi đang IDLE -> tránh chồng chéo
      if (sdcState == SDC_IDLE) {
        sdcState = SDC_MOVING_TO_93;
        lastServoMoveTime = millis();
      }
      break;

    case '8':  // Khởi động chu kỳ ServoOled
      if (oledSvState == OLED_SV_IDLE) {
        servoOled.write(SERVO_OLED_MAX);    // Quay nhanh tới 90 độ
        oledSvWaitStart = millis();
        oledSvState = OLED_SV_AT_90;
      }
      break;

    default:
      // Ký tự lạ -> bỏ qua
      break;
  }
}

// =====================================================================
// HÀM: processSerialCommand
// ---------------------------------------------------------------------
// Phân loại chuỗi nhận được theo độ dài và gọi hàm xử lý phù hợp.
//   - 1 ký tự : Relay & Servo
//   - 2 ký tự : Lệnh "44" -> xóa màn hình OLED
//   - 5 ký tự : Hiển thị 5 chữ số lên OLED
// =====================================================================
inline void processSerialCommand(const char* cmd, uint8_t len) {
  if (len == 1) {
    handleOneCharCommand(cmd[0]);
  } else if (len == 2) {
    // Chỉ chấp nhận đúng "44"
    if (cmd[0] == '4' && cmd[1] == '4') {
      displayClearScreen();
    }
  } else if (len == 5) {
    // Kiểm tra tất cả ký tự đều là chữ số
    bool allDigits = true;
    for (uint8_t i = 0; i < 5; i++) {
      if (cmd[i] < '0' || cmd[i] > '9') { allDigits = false; break; }
    }
    if (allDigits) {
      handleFiveDigitDisplay(cmd);
    }
  }
  // Các độ dài khác bỏ qua
}

// =====================================================================
// HÀM: readSerialNonBlocking
// ---------------------------------------------------------------------
// Đọc Serial từng ký tự một (không chặn luồng).
// Khi gặp '\n' thì kết thúc chuỗi và gọi hàm xử lý lệnh.
// =====================================================================
inline void readSerialNonBlocking() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    // Bỏ qua '\r' (CR) để hỗ trợ định dạng "\r\n" của Windows
    if (c == '\r') continue;

    if (c == '\n') {
      // Kết thúc chuỗi -> chốt null terminator và xử lý lệnh
      serialBuffer[serialIndex] = '\0';
      if (serialIndex > 0) {
        processSerialCommand(serialBuffer, serialIndex);
      }
      serialIndex = 0;        // Reset buffer cho lệnh tiếp theo
    } else {
      // Còn chỗ trong buffer -> lưu ký tự
      if (serialIndex < SERIAL_BUFFER_SIZE - 1) {
        serialBuffer[serialIndex++] = c;
      } else {
        // Buffer tràn -> reset, bỏ chuỗi lỗi
        serialIndex = 0;
      }
    }
  }
}

// =====================================================================
// HÀM: updateServoSDcard
// ---------------------------------------------------------------------
// State Machine non-blocking cho ServoSDcard.
// Mỗi SERVO_SDCARD_STEP_MS (32ms) tăng/giảm 1 độ -> rất chậm.
// 93 độ * 32ms ~= 3 giây.
// TUYỆT ĐỐI không cho góc vượt quá SERVO_SDCARD_MAX (93 độ).
// =====================================================================
inline void updateServoSDcard() {
  unsigned long now = millis();

  switch (sdcState) {

    case SDC_IDLE:
      // Không làm gì, chờ lệnh '7'
      break;

    case SDC_MOVING_TO_93:
      // Tăng dần góc lên 93 độ
      if (now - lastServoMoveTime >= SERVO_SDCARD_STEP_MS) {
        lastServoMoveTime = now;
        if (sdcCurrentAngle < SERVO_SDCARD_MAX) {
          sdcCurrentAngle++;
          // Chốt an toàn không cho vượt giới hạn
          if (sdcCurrentAngle > SERVO_SDCARD_MAX) {
            sdcCurrentAngle = SERVO_SDCARD_MAX;
          }
          servoSDcard.write(sdcCurrentAngle);
        } else {
          // Đã tới 93 độ -> chờ 1 giây
          sdcWaitStartTime = now;
          sdcState = SDC_WAITING_1S;
        }
      }
      break;

    case SDC_WAITING_1S:
      // Chờ đủ 1 giây bằng millis (không dùng delay)
      if (now - sdcWaitStartTime >= SERVO_SDCARD_WAIT_MS) {
        lastServoMoveTime = now;
        sdcState = SDC_MOVING_TO_HOME;
      }
      break;

    case SDC_MOVING_TO_HOME:
      // Giảm dần góc về 0 độ (HOME)
      if (now - lastServoMoveTime >= SERVO_SDCARD_STEP_MS) {
        lastServoMoveTime = now;
        if (sdcCurrentAngle > SERVO_SDCARD_HOME) {
          sdcCurrentAngle--;
          servoSDcard.write(sdcCurrentAngle);
        } else {
          // Đã về tới 0 độ -> hoàn tất chu kỳ
          sdcState = SDC_IDLE;
        }
      }
      break;
  }
}

// =====================================================================
// HÀM: updateServoOled
// ---------------------------------------------------------------------
// State Machine non-blocking cho ServoOled.
// Quay nhanh: lệnh write() là tức thời, chỉ cần dùng millis cho 0.5s chờ.
// =====================================================================
inline void updateServoOled() {
  unsigned long now = millis();

  switch (oledSvState) {

    case OLED_SV_IDLE:
      // Không làm gì, chờ lệnh '8'
      break;

    case OLED_SV_AT_90:
      // Sau khi đến 90 độ, chờ đủ 0.5 giây rồi quay về 0 độ
      if (now - oledSvWaitStart >= SERVO_OLED_WAIT_MS) {
        servoOled.write(SERVO_OLED_HOME);     // Quay nhanh về 0 độ
        oledSvState = OLED_SV_IDLE;
      }
      break;
  }
}

#endif // CONFIG_H
