/* =====================================================================
 * config.h - File header cấu hình + định nghĩa hàm cho dự án Arduino UNO R3
 * ---------------------------------------------------------------------
 * BẢNG GIÁ TRỊ NHẬN QUA CỔNG COM (kết thúc bằng ký tự '\n')
 * ---------------------------------------------------------------------
 *  Chuỗi 1 ký tự (Relay & Servo):
 *    '1' -> Bật RelayBplus              (D7 = HIGH)
 *    '2' -> Tắt RelayBplus              (D7 = LOW)
 *    '3' -> ServoFontCam về HOME (0°)   (D2)
 *    '4' -> ServoFontCam tới 45°        (D2)
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
 *         KHÔNG cần ký tự kết thúc '\n'. Lệnh được nhận biết bằng timeout
 *         50ms sau byte cuối cùng (tương thích CAPL / CANoe).
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
#define RELAY_ON HIGH // Mức logic để BẬT relay
#define RELAY_OFF LOW // Mức logic để TẮT relay

// ---------------------------------------------------------------------
// CHÂN CẮM (PIN) PHẦN CỨNG
// ---------------------------------------------------------------------
#define PIN_RELAY_BPLUS 7   // D7  -> RelayBplus
#define PIN_SERVO_FONTCAM 2 // D2  -> ServoFontCam
#define PIN_SERVO_SDCARD 5  // D5  -> ServoSDcard
#define PIN_SERVO_OLED 3    // D3  -> ServoOled

// ---------------------------------------------------------------------
// THÔNG SỐ ĐIỀU KHIỂN SERVO
// ---------------------------------------------------------------------
#define SERVO_SDCARD_HOME 0       // Vị trí gốc HOME của ServoSDcard
#define SERVO_SDCARD_MAX 93       // Góc tối đa (KHÔNG ĐƯỢC quá 93 độ)
#define SERVO_SDCARD_STEP_MS 32   // Thời gian giữa 2 bước 1 độ (ms)
                                  // 93 độ * 32ms ~= 2976ms (~3 giây)
#define SERVO_SDCARD_WAIT_MS 1000 // Thời gian chờ tại 93 độ (1 giây)

#define SERVO_OLED_HOME 0      // Vị trí gốc HOME của ServoOled
#define SERVO_OLED_MAX 90      // Góc đích của ServoOled
#define SERVO_OLED_WAIT_MS 500 // Thời gian chờ tại 90 độ (0.5 giây)

#define SERVO_FONTCAM_HOME 0 // Vị trí gốc HOME của ServoFontCam
#define SERVO_FONTCAM_MAX 45 // Góc đích của ServoFontCam khi nhận lệnh '4'

// ---------------------------------------------------------------------
// THÔNG SỐ GIAO TIẾP SERIAL
// ---------------------------------------------------------------------
#define SERIAL_BAUD 9600      // Tốc độ baud cổng COM
#define SERIAL_BUFFER_SIZE 16 // Kích thước buffer chuỗi nhận
#define SERIAL_TIMEOUT_MS 100 // Chờ tối đa 100ms sau chữ số cuối -> coi là hết lệnh (fallback)
#define SERIAL_DEBUG_ENABLE 1 // 1 = bật log debug Serial, 0 = tắt log

// ---------------------------------------------------------------------
// TỐC ĐỘ BUS I2C CHO OLED SH1106
// Đẩy lên mức cao hơn để tăng tốc độ cập nhật khung hình OLED.
// Nếu OLED hoặc dây tín hiệu không ổn định, giảm lại 400000UL.
// ---------------------------------------------------------------------
#define OLED_I2C_CLOCK_HZ 800000UL

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
enum ServoSDcardState
{
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
enum ServoOledState
{
  OLED_SV_IDLE = 0,
  OLED_SV_AT_90
};

/* =====================================================================
 * PHẦN 3: ĐỐI TƯỢNG OLED & SERVO
 * ===================================================================== */

// ---------------------------------------------------------------------
// OLED SH1106 128x64 - I2C - Phần cứng (HW I2C)
// U8G2_R2 = xoay 180 độ (vì màn hình lắp ngược)
// "_F_" = full frame buffer (1024 byte) -> gửi toàn bộ khung hình 1 lần,
//         giảm hiện tượng banding khi quay video (không bị rolling shutter
//         bắt được trạng thái cập nhật giữa chừng như "_1_" page buffer)
// Lưu ý: dùng thêm ~1KB SRAM, tổng ~1.1KB / 2KB UNO R3 -> vẫn an toàn
// ---------------------------------------------------------------------
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/U8X8_PIN_NONE);

Servo servoSDcard;  // Servo D5 - quay chậm, max 93 độ
Servo servoOled;    // Servo D3 - quay nhanh, max 90 độ
Servo servoFontCam; // Servo D2 - 2 vị trí: 0 độ (HOME) hoặc 45 độ

/* =====================================================================
 * PHẦN 4: BIẾN TOÀN CỤC
 * ===================================================================== */

// ---------------------------------------------------------------------
// Biến trạng thái ServoSDcard (Non-blocking State Machine)
// ---------------------------------------------------------------------
ServoSDcardState sdcState = SDC_IDLE;    // Khởi động: đứng yên tại HOME, chờ lệnh
int sdcCurrentAngle = SERVO_SDCARD_HOME; // Servo bắt đầu ở vị trí HOME (0°)
unsigned long lastServoMoveTime = 0;     // Mốc thời gian bước cuối
unsigned long sdcWaitStartTime = 0;      // Mốc bắt đầu chờ 1 giây

// ---------------------------------------------------------------------
// Biến trạng thái ServoOled (Non-blocking State Machine)
// ---------------------------------------------------------------------
ServoOledState oledSvState = OLED_SV_IDLE;
unsigned long oledSvWaitStart = 0; // Mốc bắt đầu chờ 0.5s

// ---------------------------------------------------------------------
// Buffer đọc Serial (Non-blocking, timeout-based)
// Tích lũy từng ký tự vào buffer, xử lý khi không có byte mới trong 50ms
// ---------------------------------------------------------------------
char serialBuffer[SERIAL_BUFFER_SIZE]; // Chuỗi đang nhận
uint8_t serialIndex = 0;               // Vị trí ghi tiếp theo
unsigned long lastByteTime = 0;        // Mốc nhận byte cuối cùng

// ---------------------------------------------------------------------
// Buffer hiển thị OLED (vì page buffer cần render lại nhiều lần)
// displayMode: 0 = trống, 1 = "YURA", 2 = 5 chữ số
// ---------------------------------------------------------------------
uint8_t displayMode = 1; // Mặc định khởi động: "YURA"
char displayP1[2] = {0}; // Phần 1 (1 chữ số)
char displayP2[3] = {0}; // Phần 2 (đã loại số 0 đầu)
char displayP3[3] = {0}; // Phần 3 (đã loại số 0 đầu)

/* =====================================================================
 * PHẦN 5: ĐỊNH NGHĨA CÁC HÀM
 * ===================================================================== */

// =====================================================================
// HÀM: logByteReceived
// ---------------------------------------------------------------------
// Log từng byte nhận từ cổng COM (mã HEX + ký tự hiển thị nếu có).
// =====================================================================
inline void logByteReceived(char c)
{
#if SERIAL_DEBUG_ENABLE
  Serial.print(F("[RX-BYTE] 0x"));
  if ((uint8_t)c < 0x10)
  {
    Serial.print('0');
  }
  Serial.print((uint8_t)c, HEX);
  Serial.print(F(" char='"));
  if (c >= 32 && c <= 126)
  {
    Serial.print(c);
  }
  else
  {
    Serial.print('.');
  }
  Serial.println(F("'"));
#else
  (void)c;
#endif
}

// =====================================================================
// HÀM: logCommandFrame
// ---------------------------------------------------------------------
// Log chuỗi lệnh hoàn chỉnh trước khi đưa vào xử lý.
// source: mô tả nguồn tách gói (delimiter hoặc timeout).
// =====================================================================
inline void logCommandFrame(const char *cmd, uint8_t len, const __FlashStringHelper *source)
{
#if SERIAL_DEBUG_ENABLE
  Serial.print(F("[RX-CMD] source="));
  Serial.print(source);
  Serial.print(F(", len="));
  Serial.print(len);
  Serial.print(F(", data=\""));
  Serial.print(cmd);
  Serial.println(F("\""));
#else
  (void)cmd;
  (void)len;
  (void)source;
#endif
}

// =====================================================================
// HÀM: logAction
// ---------------------------------------------------------------------
// Log các hành động đã thực thi (relay/servo/oled/process).
// =====================================================================
inline void logAction(const __FlashStringHelper *msg)
{
#if SERIAL_DEBUG_ENABLE
  Serial.println(msg);
#else
  (void)msg;
#endif
}

// =====================================================================
// HÀM: logOledCommand
// ---------------------------------------------------------------------
// Log chuyên biệt cho lệnh liên quan OLED nhận từ cổng COM.
// stage: giai đoạn xử lý (RECEIVED/ACCEPT/REJECT/RENDER...)
// =====================================================================
inline void logOledCommand(const __FlashStringHelper *stage, const char *cmd, uint8_t len)
{
#if SERIAL_DEBUG_ENABLE
  Serial.print(F("[OLED-COM] "));
  Serial.print(stage);
  Serial.print(F(", len="));
  Serial.print(len);
  Serial.print(F(", data=\""));
  Serial.print(cmd);
  Serial.println(F("\""));
#else
  (void)stage;
  (void)cmd;
  (void)len;
#endif
}

// =====================================================================
// HÀM: renderOled
// ---------------------------------------------------------------------
// Vẽ nội dung lên OLED theo trạng thái displayMode hiện tại.
// Chế độ full buffer (_F_): vẽ toàn bộ vào RAM trước, sau đó gửi
// 1 lần bằng sendBuffer() -> màn hình chuyển trạng thái tức thời,
// không bị rolling shutter camera bắt được trạng thái "nửa vời".
//   displayMode = 0 -> để trống
//   displayMode = 1 -> hiển thị "YURA" to ở giữa
//   displayMode = 2 -> hiển thị 5 chữ số (theo quy tắc 3 phần)
// =====================================================================
inline void renderOled()
{
#if SERIAL_DEBUG_ENABLE
  Serial.print(F("[OLED-RENDER] mode="));
  Serial.println(displayMode);
#endif

  u8g2.clearBuffer(); // Xóa frame buffer trong RAM trước khi vẽ

  if (displayMode == 1)
  {
#if SERIAL_DEBUG_ENABLE
    Serial.println(F("[OLED-RENDER] draw boot text: YURA"));
#endif

    // Hiển thị chữ "YURA" to ở giữa màn hình
    u8g2.setFont(u8g2_font_logisoso28_tr); // Font cao 28px (có chữ cái)
    const char *msg = "YURA";
    int w = u8g2.getStrWidth(msg);
    int x = (128 - w) / 2; // Căn giữa ngang
    int y = 32 + 14;       // Căn giữa dọc (baseline)
    u8g2.drawStr(x, y, msg);
  }
  else if (displayMode == 2)
  {
#if SERIAL_DEBUG_ENABLE
    Serial.print(F("[OLED-RENDER] line1-left='"));
    Serial.print(displayP1);
    Serial.print(F("', line1-right='"));
    Serial.print(displayP2);
    Serial.print(F("', line2-center='"));
    Serial.print(displayP3);
    Serial.println(F("'"));
#endif

    // Hiển thị 5 chữ số: P1 trái, P2 phải (dòng 1), P3 giữa (dòng 2)
    u8g2.setFont(u8g2_font_logisoso28_tn); // Font số cao 28px

    // Dòng 1: baseline y = 28
    int y1 = 28;
    u8g2.drawStr(0, y1, displayP1); // P1 sát trái
    int wP2 = u8g2.getStrWidth(displayP2);
    u8g2.drawStr(128 - wP2, y1, displayP2); // P2 sát phải

    // Dòng 2: baseline y = 63 (sát đáy)
    int y2 = 63;
    int wP3 = u8g2.getStrWidth(displayP3);
    int xP3 = (128 - wP3) / 2; // Căn giữa
    u8g2.drawStr(xP3, y2, displayP3);
  }
  // displayMode = 0 -> clearBuffer() đã xóa -> màn hình trống

  // Gửi toàn bộ 1024 byte frame buffer xuống SH1106 trong 1 giao dịch I2C
  // -> camera không thể bắt được trạng thái "đang vẽ giữa chừng"
  u8g2.sendBuffer();
}

// =====================================================================
// HÀM: displayBootMessage - Hiển thị "YURA" to ở giữa khi khởi động.
// =====================================================================
inline void displayBootMessage()
{
  displayMode = 1;
  logAction(F("[OLED] Show boot message: YURA"));
  renderOled();
}

// =====================================================================
// HÀM: displayClearScreen - Xóa toàn bộ màn hình OLED (lệnh "44").
// =====================================================================
inline void displayClearScreen()
{
  displayMode = 0;
  logAction(F("[OLED] Clear screen (command 44)"));
  logOledCommand(F("ACTION_CLEAR"), "44", 2);
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
inline void handleFiveDigitDisplay(const char *digits)
{
  // P1: 1 ký tự
  displayP1[0] = digits[0];
  displayP1[1] = '\0';

  // P2: bỏ số 0 đầu nếu có
  if (digits[1] == '0')
  {
    displayP2[0] = digits[2];
    displayP2[1] = '\0';
  }
  else
  {
    displayP2[0] = digits[1];
    displayP2[1] = digits[2];
    displayP2[2] = '\0';
  }

  // P3: bỏ số 0 đầu nếu có
  if (digits[3] == '0')
  {
    displayP3[0] = digits[4];
    displayP3[1] = '\0';
  }
  else
  {
    displayP3[0] = digits[3];
    displayP3[1] = digits[4];
    displayP3[2] = '\0';
  }

  displayMode = 2;
#if SERIAL_DEBUG_ENABLE
  Serial.print(F("[OLED] 5-digit parsed -> P1='"));
  Serial.print(displayP1);
  Serial.print(F("', P2='"));
  Serial.print(displayP2);
  Serial.print(F("', P3='"));
  Serial.print(displayP3);
  Serial.println(F("'"));
#endif
  renderOled();
}

// =====================================================================
// HÀM: handleOneCharCommand - Xử lý lệnh 1 ký tự cho Relay và Servo.
// =====================================================================
inline void handleOneCharCommand(char c)
{
#if SERIAL_DEBUG_ENABLE
  Serial.print(F("[PROCESS] One-char command='"));
  Serial.print(c);
  Serial.println(F("'"));
#endif

  switch (c)
  {
  case '1': // Bật RelayBplus (D7)
    digitalWrite(PIN_RELAY_BPLUS, RELAY_ON);
    logAction(F("[ACT] RelayBplus -> ON (D7)"));
    break;
  case '2': // Tắt RelayBplus (D7)
    digitalWrite(PIN_RELAY_BPLUS, RELAY_OFF);
    logAction(F("[ACT] RelayBplus -> OFF (D7)"));
    break;

  case '3': // ServoFontCam quay về HOME (0 độ)
    servoFontCam.write(SERVO_FONTCAM_HOME);
    logAction(F("[ACT] ServoFontCam -> HOME 0 deg (D2)"));
    break;
  case '4': // ServoFontCam quay tới 45 độ
    servoFontCam.write(SERVO_FONTCAM_MAX);
    logAction(F("[ACT] ServoFontCam -> 45 deg (D2)"));
    break;

  case '7': // Khởi động chu kỳ ServoSDcard
    // Chỉ cho phép chu kỳ mới khi đang IDLE -> tránh chồng chéo
    if (sdcState == SDC_IDLE)
    {
      sdcState = SDC_MOVING_TO_93;
      lastServoMoveTime = millis();
      logAction(F("[ACT] ServoSDcard cycle START -> moving to 93 deg"));
    }
    else
    {
#if SERIAL_DEBUG_ENABLE
      Serial.print(F("[WARN] Command '7' ignored, ServoSDcard busy, state="));
      Serial.println((int)sdcState);
#endif
    }
    break;

  case '8': // Khởi động chu kỳ ServoOled
#if SERIAL_DEBUG_ENABLE
    Serial.print(F("[ACT] Command '8' received, oledSvState="));
    Serial.println((int)oledSvState);
#endif
    if (oledSvState == OLED_SV_IDLE)
    {
      servoOled.write(SERVO_OLED_MAX); // Quay nhanh tới 90 độ
      oledSvWaitStart = millis();
      oledSvState = OLED_SV_AT_90;
      logAction(F("[ACT] ServoOled -> 90 deg (D3), wait 500ms"));
    }
    else
    {
      logAction(F("[WARN] Command '8' ignored, ServoOled still busy"));
    }
    break;

  default:
    // Ký tự lạ -> bỏ qua
    logAction(F("[WARN] Unknown one-char command, ignored"));
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
inline void processSerialCommand(const char *cmd, uint8_t len)
{
#if SERIAL_DEBUG_ENABLE
  Serial.print(F("[PROCESS] Dispatch command len="));
  Serial.println(len);
#endif

  if (len == 1)
  {
    handleOneCharCommand(cmd[0]);
  }
  else if (len == 2)
  {
    logOledCommand(F("RECEIVED_LEN2"), cmd, len);

    // Chỉ chấp nhận đúng "44"
    if (cmd[0] == '4' && cmd[1] == '4')
    {
      logOledCommand(F("ACCEPT_CLEAR_44"), cmd, len);
      displayClearScreen();
    }
    else
    {
      logOledCommand(F("REJECT_LEN2_NOT_44"), cmd, len);
      logAction(F("[WARN] 2-char command is not '44', ignored"));
    }
  }
  else if (len == 5)
  {
    logOledCommand(F("RECEIVED_LEN5"), cmd, len);

    // Kiểm tra tất cả ký tự đều là chữ số
    bool allDigits = true;
    for (uint8_t i = 0; i < 5; i++)
    {
      if (cmd[i] < '0' || cmd[i] > '9')
      {
        allDigits = false;
        break;
      }
    }
    if (allDigits)
    {
      logOledCommand(F("ACCEPT_LEN5_DIGITS"), cmd, len);
      handleFiveDigitDisplay(cmd);
    }
    else
    {
      logOledCommand(F("REJECT_LEN5_NON_DIGIT"), cmd, len);
      logAction(F("[WARN] 5-char command has non-digit, ignored"));
    }
  }
  else
  {
    logAction(F("[WARN] Unsupported command length, ignored"));
  }
  // Các độ dài khác bỏ qua
}

// =====================================================================
// HÀM: readSerialNonBlocking
// ---------------------------------------------------------------------
// Đọc Serial từng ký tự một (không chặn luồng).
// Logic tách ký tự theo kiểu reference code đã chạy thực tế:
//   - Ký tự là chữ số ('0'..'9') -> tích lũy vào buffer
//   - Ký tự KHÔNG phải số (\n, \r, space, ...) -> kích hoạt xử lý NGAY
//     mà KHÔNG thêm ký tự đó vào buffer (tránh lỗi len sai)
// Bug cũ: mọi ký tự đều nhét vào buffer nên "44\n" cho len=3 thay vì 2,
// "10203\n" cho len=6 thay vì 5 -> không khớp case nào -> bị bỏ qua.
// Timeout 100ms vẫn giữ làm fallback cho tool gửi không có ký tự kết thúc.
// =====================================================================
inline void readSerialNonBlocking()
{
  while (Serial.available() > 0)
  {
    char c = (char)Serial.read();
    logByteReceived(c);

    if (c >= '0' && c <= '9')
    {
      // Chữ số -> tích lũy vào buffer, cập nhật mốc thời gian
      lastByteTime = millis();
      if (serialIndex < SERIAL_BUFFER_SIZE - 1)
      {
        serialBuffer[serialIndex++] = c;
      }
      else
      {
        logAction(F("[WARN] Serial buffer overflow -> reset buffer"));
        serialIndex = 0; // Buffer tràn -> reset, bỏ chuỗi lỗi
      }
    }
    else
    {
      // Không phải số (\n, \r, space...) -> kích hoạt xử lý ngay nếu có dữ liệu
      if (serialIndex > 0)
      {
        serialBuffer[serialIndex] = '\0';
        logCommandFrame(serialBuffer, serialIndex, F("delimiter"));
        processSerialCommand(serialBuffer, serialIndex);
        serialIndex = 0;
      }
      else
      {
        logAction(F("[RX] Delimiter received while buffer empty, ignored"));
      }
    }
  }

  // --- Timeout fallback 100ms: tool gửi không kèm ký tự kết thúc ---
  if (serialIndex > 0 && (millis() - lastByteTime >= SERIAL_TIMEOUT_MS))
  {
    serialBuffer[serialIndex] = '\0';
    logCommandFrame(serialBuffer, serialIndex, F("timeout"));
    processSerialCommand(serialBuffer, serialIndex);
    serialIndex = 0;
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
inline void updateServoSDcard()
{
  unsigned long now = millis();

  switch (sdcState)
  {

  case SDC_IDLE:
    // Không làm gì, chờ lệnh '7'
    break;

  case SDC_MOVING_TO_93:
    // Tăng dần góc lên 93 độ
    if (now - lastServoMoveTime >= SERVO_SDCARD_STEP_MS)
    {
      lastServoMoveTime = now;
      if (sdcCurrentAngle < SERVO_SDCARD_MAX)
      {
        sdcCurrentAngle++;
        // Chốt an toàn không cho vượt giới hạn
        if (sdcCurrentAngle > SERVO_SDCARD_MAX)
        {
          sdcCurrentAngle = SERVO_SDCARD_MAX;
        }
        servoSDcard.write(sdcCurrentAngle);
      }
      else
      {
        // Đã tới 93 độ -> chờ 1 giây
        sdcWaitStartTime = now;
        sdcState = SDC_WAITING_1S;
        logAction(F("[ACT] ServoSDcard reached 93 deg -> waiting 1s"));
      }
    }
    break;

  case SDC_WAITING_1S:
    // Chờ đủ 1 giây bằng millis (không dùng delay)
    if (now - sdcWaitStartTime >= SERVO_SDCARD_WAIT_MS)
    {
      lastServoMoveTime = now;
      sdcState = SDC_MOVING_TO_HOME;
      logAction(F("[ACT] ServoSDcard wait done -> moving to HOME"));
    }
    break;

  case SDC_MOVING_TO_HOME:
    // Giảm dần góc về 0 độ (HOME)
    if (now - lastServoMoveTime >= SERVO_SDCARD_STEP_MS)
    {
      lastServoMoveTime = now;
      if (sdcCurrentAngle > SERVO_SDCARD_HOME)
      {
        sdcCurrentAngle--;
        servoSDcard.write(sdcCurrentAngle);
      }
      else
      {
        // Đã về tới 0 độ -> hoàn tất chu kỳ
        sdcState = SDC_IDLE;
        logAction(F("[ACT] ServoSDcard cycle DONE at HOME"));
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
inline void updateServoOled()
{
  unsigned long now = millis();

  switch (oledSvState)
  {

  case OLED_SV_IDLE:
    // Không làm gì, chờ lệnh '8'
    break;

  case OLED_SV_AT_90:
    // Sau khi đến 90 độ, chờ đủ 0.5 giây rồi quay về 0 độ
    if (now - oledSvWaitStart >= SERVO_OLED_WAIT_MS)
    {
      servoOled.write(SERVO_OLED_HOME); // Quay nhanh về 0 độ
      oledSvState = OLED_SV_IDLE;
      logAction(F("[ACT] ServoOled returned to HOME 0 deg"));
    }
    break;
  }
}

#endif // CONFIG_H
