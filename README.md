# m5tg-life-counter

A portable Magic: The Gathering life counter built on M5StickC Plus2.

## Features

- **Game Modes**: Standard (20), Commander (40), or Custom life totals (1-255)
- **2 Players**: Independent life tracking with turn timer
- **Player Themes**: 5 color themes based on MTG mana colors (Plains, Island, Swamp, Mountain, Forest)
- **Dice Roller**: d4, d6, d8, d10, d12, d20, d100
- **Coin Flip**: Quick heads/tails with animation
- **Shake Controls**: Shake the device to switch players, roll dice, or flip coins
- **Face Down Pause**: Place the device face down to pause the game and save battery
- **Power Saving**: Automatically reduces brightness and CPU speed at low battery
- **Auto Shutdown**: Configurable idle and in-game timeouts
- **Persistent Stats**: Tracks total matches, wins, playtime, dice rolls, and coin flips
- **Victory Animation**: Animated celebration when a player wins
- **Diagnostics**: Battery info, system info, temperature, IMU status, and hardware tests

## Controls

| Input | Action |
|-------|--------|
| Button A (front) | +1 life / select / adjust up |
| Button B (side) | -1 life / navigate / adjust down |
| Long press A | +5 life (repeat) |
| Long press B | -5 life (repeat) |
| Power (click) | Open menu / go back |
| Power (hold 6s) | Power off (hardware) |
| Shake device | Switch player / roll dice / flip coin |

## Hardware

- **Board**: [M5StickC Plus2](https://docs.m5stack.com/en/core/M5StickC%20PLUS2)
- **MCU**: ESP32-PICO-V3-02
- **Display**: 135x240 TFT (ST7789v2)
- **IMU**: MPU6886 (shake detection, face down detection)
- **Battery**: 120mAh LiPo

## Dependencies

- [M5Unified](https://github.com/m5stack/M5Unified) (install via Arduino Library Manager)

## Building

1. Install [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/)
2. Add the M5Stack board package (ESP32)
3. Install the **M5Unified** library
4. Open `mtg-life-counter.ino`
5. Select board **M5StickC Plus2** and upload
