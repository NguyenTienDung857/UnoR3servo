# Repository Guidelines

## Project Structure & Module Organization

This is a PlatformIO Arduino Uno project. The active firmware lives in `src/main.cpp`; it handles serial commands, relay control, four servos, and an SH1106 I2C OLED via U8g2. Shared headers should go in `include/` when logic grows beyond one file. Project-specific libraries belong under `lib/<library_name>/`, preferably with their own `src/` folder. PlatformIO unit tests should be placed in `test/`. Hardware behavior and the serial command map are also documented in `Claude.md`.

## Build, Test, and Development Commands

- `pio run -e uno`: compile the firmware for Arduino Uno.
- `pio run -e uno -t upload`: build and upload to the connected Uno.
- `pio device monitor -b 9600`: open the serial monitor at the firmware baud rate.
- `pio test -e uno`: run PlatformIO tests when tests are added under `test/`.

If `pio` is not on `PATH`, use the local PlatformIO executable, for example `C:\Users\tiend\.platformio\penv\Scripts\pio.exe run -e uno`.

## Coding Style & Naming Conventions

Use C++ for Arduino with two-space indentation, braces on their own control blocks as already shown in `src/main.cpp`, and `constexpr` for pin numbers, timing values, buffer sizes, and servo limits. Keep pin constants named `PIN_*`, timing constants named `*_MS`, and enum state machines scoped with `enum class`. Prefer non-blocking logic based on `millis()`; avoid adding long `delay()` calls in `loop()`.

## Testing Guidelines

There are no project tests yet. For pure helpers, add PlatformIO tests in `test/` and name them by behavior, such as `test_serial_command_parser.cpp`. Hardware changes must be verified on the Uno with serial monitor logs enabled. Check relay default state, FontCam and RearCam home/work angles, OLED clear/render commands, and serial timeout behavior.

## Commit & Pull Request Guidelines

This checkout has no Git history available, so use short imperative commit messages such as `fix: clamp sdcard servo angle` or `docs: update serial command map`. Pull requests should describe the hardware behavior changed, list tested commands, include the PlatformIO build result, and mention any pin, baud rate, OLED, or servo-limit changes.

## Agent-Specific Instructions

Do not raise `SERVO_SDCARD_MAX_ANGLE` beyond the documented mechanical limit without explicit hardware validation. Keep serial debug logs behind `SERIAL_DEBUG_ENABLE`, and update `Claude.md` when command behavior or pin assignments change.
