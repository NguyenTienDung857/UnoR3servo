#include <Arduino.h>
#include <Servo.h>
#include <U8g2lib.h>

// Toggle serial debug logs: 1 = enable, 0 = disable
#define SERIAL_DEBUG_ENABLE 1

constexpr uint8_t PIN_RELAY_BPLUS = 7;
constexpr uint8_t PIN_SERVO_FONTCAM = 2;
constexpr uint8_t PIN_SERVO_REARCAM = 8;
constexpr uint8_t PIN_SERVO_SDCARD = 5;
constexpr uint8_t PIN_SERVO_OLED = 3;

constexpr uint8_t RELAY_ON = HIGH;
constexpr uint8_t RELAY_OFF = LOW;

constexpr uint32_t SERIAL_BAUDRATE = 9600;
constexpr uint32_t RX_TIMEOUT_MS = 35;
constexpr size_t RX_BUFFER_SIZE = 16;

constexpr uint8_t SERVO_FONTCAM_HOME_ANGLE = 0;
constexpr uint8_t SERVO_FONTCAM_WORK_ANGLE = 45;

constexpr uint8_t SERVO_REARCAM_HOME_ANGLE = 0;
constexpr uint8_t SERVO_REARCAM_WORK_ANGLE = 45;
constexpr uint8_t SERVO_ANGLE_UNKNOWN = 255;

constexpr uint8_t SERVO_SDCARD_MIN_ANGLE = 0;
constexpr uint8_t SERVO_SDCARD_MAX_ANGLE = 93;
constexpr uint32_t SERVO_SDCARD_STEP_MS = 32;
constexpr uint32_t SERVO_SDCARD_WAIT_MS = 1000;

constexpr uint8_t SERVO_OLED_MIN_ANGLE = 0;
constexpr uint8_t SERVO_OLED_SWING_ANGLE = 90;
constexpr uint32_t SERVO_OLED_WAIT_MS = 500;

constexpr uint32_t OLED_I2C_CLOCK_HZ = 400000UL;
constexpr uint8_t OLED_DISPLAY_CLOCK = 0xF0;
constexpr uint8_t OLED_CONTRAST = 255;

Servo servoFontCam;
Servo servoRearCam;
Servo servoSDcard;
Servo servoOled;

// Common 1.3" I2C OLED (SH1106), rotated 180 degrees by using U8G2_R2.
// Full-buffer mode draws the frame in SRAM before sending it to reduce camera-visible page tearing.
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R2, U8X8_PIN_NONE);

enum class SDCardCycleState : uint8_t
{
  IDLE,
  MOVING_UP,
  WAITING_AT_TOP,
  MOVING_DOWN
};

enum class OledServoState : uint8_t
{
  IDLE,
  WAITING_RETURN
};

SDCardCycleState sdcardState = SDCardCycleState::IDLE;
OledServoState oledServoState = OledServoState::IDLE;

uint8_t sdcardAngle = SERVO_SDCARD_MIN_ANGLE;
uint8_t rearCamAngle = SERVO_ANGLE_UNKNOWN;

uint32_t sdcardLastStepMs = 0;
uint32_t sdcardWaitStartMs = 0;
uint32_t oledServoStartMs = 0;

char rxBuffer[RX_BUFFER_SIZE] = {0};
size_t rxLen = 0;
uint32_t lastRxByteMs = 0;

#if SERIAL_DEBUG_ENABLE
void logLine(const __FlashStringHelper *tag, const __FlashStringHelper *text)
{
  Serial.print(tag);
  Serial.println(text);
}

void logLine(const __FlashStringHelper *tag, const char *text)
{
  Serial.print(tag);
  Serial.println(text);
}

void logRxByte(uint8_t b)
{
  Serial.print(F("[RX-BYTE] 0x"));
  if (b < 0x10)
  {
    Serial.print('0');
  }
  Serial.print(b, HEX);
  if (b >= 32 && b <= 126)
  {
    Serial.print(F(" ('"));
    Serial.write(static_cast<char>(b));
    Serial.println(F("')"));
  }
  else
  {
    Serial.println();
  }
}
#else
void logLine(const __FlashStringHelper *, const __FlashStringHelper *) {}
void logLine(const __FlashStringHelper *, const char *) {}
void logRxByte(uint8_t) {}
#endif

void relaySet(bool on)
{
  digitalWrite(PIN_RELAY_BPLUS, on ? RELAY_ON : RELAY_OFF);
  logLine(F("[ACT] "), on ? F("RelayBplus ON") : F("RelayBplus OFF"));
}

void setRearCamAngle(uint8_t targetAngle)
{
  if (rearCamAngle == targetAngle)
  {
    logLine(F("[ACT] "), targetAngle == SERVO_REARCAM_HOME_ANGLE ? F("ServoRearCam already at 0 deg") : F("ServoRearCam already at 45 deg"));
    return;
  }

  servoRearCam.write(targetAngle);
  rearCamAngle = targetAngle;
  logLine(F("[ACT] "), targetAngle == SERVO_REARCAM_HOME_ANGLE ? F("ServoRearCam -> 0 deg") : F("ServoRearCam -> 45 deg"));
}

void configureOledForCamera()
{
  oled.setBusClock(OLED_I2C_CLOCK_HZ);
  oled.begin();
  oled.setContrast(OLED_CONTRAST);
  oled.sendF("ca", 0x0D5, OLED_DISPLAY_CLOCK);
  logLine(F("[OLED-CONFIG] "), F("full-buffer, i2c=400kHz, display-clock=0xF0, contrast=255"));
}

void oledClearScreen()
{
  oled.clearBuffer();
  oled.sendBuffer();
  logLine(F("[OLED-RENDER] "), F("mode=clear"));
}

bool isAllDigits(const char *s, size_t len)
{
  for (size_t i = 0; i < len; ++i)
  {
    if (s[i] < '0' || s[i] > '9')
    {
      return false;
    }
  }
  return true;
}

void renderOledDigits(uint8_t p1, uint8_t p2, uint8_t p3)
{
  char p1Text[2];
  char p2Text[3];
  char p3Text[3];

  snprintf(p1Text, sizeof(p1Text), "%u", p1);
  snprintf(p2Text, sizeof(p2Text), "%u", p2);
  snprintf(p3Text, sizeof(p3Text), "%u", p3);

  oled.clearBuffer();
  oled.setFontPosTop();

  oled.setFont(u8g2_font_logisoso26_tn);
  const int topY = 0;
  const int p2Width = oled.getStrWidth(p2Text);
  const int p2X = (128 - p2Width < 0) ? 0 : 128 - p2Width;
  oled.drawStr(0, topY, p1Text);
  oled.drawStr(p2X, topY, p2Text);

  oled.setFont(u8g2_font_logisoso38_tn);
  const int p3Width = oled.getStrWidth(p3Text);
  const int p3XRaw = (128 - p3Width) / 2;
  const int p3X = (p3XRaw < 0) ? 0 : p3XRaw;
  const int bottomY = 26;
  oled.drawStr(p3X, bottomY, p3Text);

  oled.sendBuffer();

#if SERIAL_DEBUG_ENABLE
  Serial.print(F("[OLED-RENDER] mode=digits p1="));
  Serial.print(p1);
  Serial.print(F(" p2="));
  Serial.print(p2);
  Serial.print(F(" p3="));
  Serial.println(p3);
#endif
}

bool startSDCardCycle()
{
  if (sdcardState != SDCardCycleState::IDLE)
  {
    logLine(F("[WARN] "), F("ServoSDcard cycle already running, command ignored"));
    return false;
  }

  sdcardAngle = SERVO_SDCARD_MIN_ANGLE;
  servoSDcard.write(sdcardAngle);
  sdcardState = SDCardCycleState::MOVING_UP;
  sdcardLastStepMs = millis();
  logLine(F("[ACT] "), F("ServoSDcard cycle started"));
  return true;
}

void updateSDCardCycle()
{
  const uint32_t now = millis();

  switch (sdcardState)
  {
  case SDCardCycleState::IDLE:
    break;

  case SDCardCycleState::MOVING_UP:
    if (now - sdcardLastStepMs >= SERVO_SDCARD_STEP_MS)
    {
      sdcardLastStepMs = now;
      if (sdcardAngle < SERVO_SDCARD_MAX_ANGLE)
      {
        ++sdcardAngle;
        servoSDcard.write(sdcardAngle);
      }
      else
      {
        sdcardState = SDCardCycleState::WAITING_AT_TOP;
        sdcardWaitStartMs = now;
        logLine(F("[ACT] "), F("ServoSDcard reached max angle, waiting"));
      }
    }
    break;

  case SDCardCycleState::WAITING_AT_TOP:
    if (now - sdcardWaitStartMs >= SERVO_SDCARD_WAIT_MS)
    {
      sdcardState = SDCardCycleState::MOVING_DOWN;
      sdcardLastStepMs = now;
      logLine(F("[ACT] "), F("ServoSDcard returning to home"));
    }
    break;

  case SDCardCycleState::MOVING_DOWN:
    if (now - sdcardLastStepMs >= SERVO_SDCARD_STEP_MS)
    {
      sdcardLastStepMs = now;
      if (sdcardAngle > SERVO_SDCARD_MIN_ANGLE)
      {
        --sdcardAngle;
        servoSDcard.write(sdcardAngle);
      }
      else
      {
        sdcardState = SDCardCycleState::IDLE;
        logLine(F("[ACT] "), F("ServoSDcard cycle completed"));
      }
    }
    break;
  }
}

bool triggerOledServoCycle()
{
  if (oledServoState != OledServoState::IDLE)
  {
    logLine(F("[WARN] "), F("ServoOled cycle already running, command ignored"));
    return false;
  }

  servoOled.write(SERVO_OLED_SWING_ANGLE);
  oledServoState = OledServoState::WAITING_RETURN;
  oledServoStartMs = millis();
  logLine(F("[ACT] "), F("ServoOled moved to 90 deg"));
  return true;
}

void updateOledServoCycle()
{
  if (oledServoState == OledServoState::WAITING_RETURN)
  {
    const uint32_t now = millis();
    if (now - oledServoStartMs >= SERVO_OLED_WAIT_MS)
    {
      servoOled.write(SERVO_OLED_MIN_ANGLE);
      oledServoState = OledServoState::IDLE;
      logLine(F("[ACT] "), F("ServoOled returned to 0 deg"));
    }
  }
}

void processCommand(const char *cmd)
{
  const size_t len = strlen(cmd);

#if SERIAL_DEBUG_ENABLE
  Serial.print(F("[PROCESS] cmd=\""));
  Serial.print(cmd);
  Serial.println(F("\""));
#endif

  if (len == 1)
  {
    switch (cmd[0])
    {
    case '1':
      relaySet(true);
      return;
    case '2':
      relaySet(false);
      return;
    case '3':
      servoFontCam.write(SERVO_FONTCAM_HOME_ANGLE);
      logLine(F("[ACT] "), F("ServoFontCam -> 0 deg"));
      return;
    case '4':
      servoFontCam.write(SERVO_FONTCAM_WORK_ANGLE);
      logLine(F("[ACT] "), F("ServoFontCam -> 45 deg"));
      return;
    case '5':
      logLine(F("[CMD] "), F("5 -> ServoRearCam HOME 0 deg"));
      setRearCamAngle(SERVO_REARCAM_HOME_ANGLE);
      return;
    case '6':
      logLine(F("[CMD] "), F("6 -> ServoRearCam WORK 45 deg"));
      setRearCamAngle(SERVO_REARCAM_WORK_ANGLE);
      return;
    case '7':
      startSDCardCycle();
      return;
    case '8':
      triggerOledServoCycle();
      return;
    default:
      logLine(F("[WARN] "), F("Unknown 1-char command"));
      return;
    }
  }

  if (len == 2 && strcmp(cmd, "44") == 0)
  {
    logLine(F("[OLED-COM] "), F("command=44 accepted -> clear"));
    oledClearScreen();
    return;
  }

  if (len == 5)
  {
    if (!isAllDigits(cmd, len))
    {
      logLine(F("[OLED-COM] "), F("5-char command rejected (not all digits)"));
      return;
    }

    const uint8_t p1 = static_cast<uint8_t>(cmd[0] - '0');
    const uint8_t p2 = static_cast<uint8_t>((cmd[1] - '0') * 10 + (cmd[2] - '0'));
    const uint8_t p3 = static_cast<uint8_t>((cmd[3] - '0') * 10 + (cmd[4] - '0'));

    logLine(F("[OLED-COM] "), F("5-digit command accepted"));
    renderOledDigits(p1, p2, p3);
    return;
  }

  logLine(F("[WARN] "), F("Unsupported command length or format"));
}

void finalizeRxCommand(const char *reason)
{
  if (rxLen == 0)
  {
    return;
  }

  rxBuffer[rxLen] = '\0';

#if SERIAL_DEBUG_ENABLE
  Serial.print(F("[RX-CMD] source="));
  Serial.print(reason);
  Serial.print(F(" cmd=\""));
  Serial.print(rxBuffer);
  Serial.println(F("\""));
#endif

  processCommand(rxBuffer);
  rxLen = 0;
}

void readSerialCommands()
{
  while (Serial.available() > 0)
  {
    const uint8_t raw = static_cast<uint8_t>(Serial.read());
    logRxByte(raw);

    if (raw == '\r' || raw == '\n')
    {
      finalizeRxCommand("delimiter");
      continue;
    }

    if (rxLen >= RX_BUFFER_SIZE - 1)
    {
      logLine(F("[WARN] "), F("RX buffer overflow, dropping current command"));
      rxLen = 0;
    }

    rxBuffer[rxLen++] = static_cast<char>(raw);
    lastRxByteMs = millis();
  }

  if (rxLen > 0)
  {
    const uint32_t now = millis();
    if (now - lastRxByteMs >= RX_TIMEOUT_MS)
    {
      finalizeRxCommand("timeout");
    }
  }
}

void setup()
{
  Serial.begin(SERIAL_BAUDRATE);

  pinMode(PIN_RELAY_BPLUS, OUTPUT);
  digitalWrite(PIN_RELAY_BPLUS, RELAY_OFF);

  servoSDcard.attach(PIN_SERVO_SDCARD);
  servoOled.attach(PIN_SERVO_OLED);
  servoFontCam.attach(PIN_SERVO_FONTCAM);
  servoRearCam.attach(PIN_SERVO_REARCAM);
  logLine(F("[SETUP] "), F("ServoRearCam attached on D8"));

  servoSDcard.write(SERVO_SDCARD_MIN_ANGLE);
  servoOled.write(SERVO_OLED_MIN_ANGLE);
  servoFontCam.write(SERVO_FONTCAM_HOME_ANGLE);
  logLine(F("[SETUP] "), F("ServoFontCam home=0 deg"));
  setRearCamAngle(SERVO_REARCAM_HOME_ANGLE);

  configureOledForCamera();
  oledClearScreen();

  logLine(F("[ACT] "), F("Boot completed: Serial 9600, Relay OFF, 4 servos attached"));
}

void loop()
{
  readSerialCommands();
  updateSDCardCycle();
  updateOledServoCycle();
}
