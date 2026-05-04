# Repository Guidelines

## Project Structure & Module Organization

This is a PlatformIO Arduino UNO R3 firmware project. `platformio.ini` defines the `uno` environment, Arduino framework, 9600 baud monitor speed, and Servo/U8g2 dependencies. Keep `src/main.cpp` focused on `setup()` and `loop()`. Hardware constants, globals, state machines, and helpers currently live in `include/config.h`. Use `lib/` only for project-local libraries and `test/` for PlatformIO test runner cases. `.pio/` and generated VS Code files are build/editor artifacts and should remain untracked.

## Build, Test, and Development Commands

Install PlatformIO CLI or the VS Code PlatformIO extension before running these commands.

- `pio run -e uno`: compile the UNO firmware and resolve dependencies.
- `pio run -e uno -t upload`: build and upload to the connected Arduino UNO.
- `pio device monitor -b 9600`: open the serial monitor at the configured baud rate.
- `pio test -e uno`: run PlatformIO unit tests when test cases exist under `test/`.

## Coding Style & Naming Conventions

Use Arduino C++ with two-space indentation and function braces on their own line. Prefer descriptive names: constants/macros in `UPPER_SNAKE_CASE` such as `PIN_SERVO_OLED`, enum values with subsystem prefixes such as `SDC_IDLE`, and functions in lower camel case such as `readSerialNonBlocking()`. Keep runtime timing non-blocking with `millis()`; avoid `delay()`. Put pin mappings, angle limits, serial settings, and debug toggles in `include/config.h`.

## Testing Guidelines

There are no committed test cases yet beyond `test/README`. Add PlatformIO tests under folders such as `test/test_serial_commands/test_main.cpp`. Focus tests on command parsing, OLED display state decisions, relay actions, and servo state transitions. Before review, run `pio run -e uno`; run `pio test -e uno` for testable logic changes.

## Commit & Pull Request Guidelines

Recent commits use short Vietnamese progress messages. Keep commits concise, but make the changed behavior clear, for example `fix servo oled return timing` or `update OLED 44 command`. Pull requests should include the firmware behavior changed, affected pins or serial commands, validation performed, and any hardware setup needed to reproduce the result.

## Hardware & Configuration Notes

Document any pin, relay polarity, servo angle, I2C clock, or serial protocol change in both code comments and `Claude.md`. Do not commit local build output, board cache files, or machine-specific editor settings.
