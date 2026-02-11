#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <pgmspace.h>

// === App Info ===
#define APP_VERSION "v1.0.0"
#define APP_DEVICE "M5StickC PLUS 2"

// === Screen ===
#define SCREEN_W 240
#define SCREEN_H 135
#define DEFAULT_BRIGHTNESS 128

// === Game Modes ===
#define LIFE_STANDARD  20
#define LIFE_COMMANDER 40
#define LIFE_MIN       1
#define LIFE_MAX       255
#define LIFE_DEFAULT   30
#define MAX_PLAYERS    2
#define HISTORY_SIZE   10

// === Input Timing (ms) ===
#define LONG_PRESS_MS     500
#define REPEAT_DELAY_MS   150

// === IMU ===
#define SHAKE_THRESHOLD     1.75f
#define SHAKE_COOLDOWN_MS   1000
#define FACE_DOWN_THRESHOLD -0.7f
#define ORIENTATION_CHECK_INTERVAL_MS 500

// === Power / Auto Shutdown ===
#define SHUTDOWN_IDLE_COUNT  3
#define SHUTDOWN_GAME_COUNT  3
const unsigned long SHUTDOWN_IDLE_MS[] PROGMEM = { 300000, 600000, 900000 };
const unsigned long SHUTDOWN_GAME_MS[] PROGMEM = { 900000, 1500000, 2100000 };
const uint8_t SHUTDOWN_IDLE_MIN[] PROGMEM = { 5, 10, 15 };
const uint8_t SHUTDOWN_GAME_MIN[] PROGMEM = { 15, 25, 35 };

// === Power Saving Mode ===
#define POWER_SAVE_BATTERY_THRESHOLD 20
#define POWER_SAVE_BRIGHTNESS        76
#define POWER_SAVE_CPU_MHZ           80
#define POWER_CHECK_INTERVAL_MS      5000

// === Audio ===
#define SPEAKER_VOLUME    120
#define TONE_LIFE_UP      880
#define TONE_LIFE_DOWN    220
#define TONE_DICE_ROLL    1200
#define TONE_COIN_FLIP    600
#define TONE_CONFIRM      1047
#define TONE_DEFEAT       147
#define TONE_DURATION     80

// === Fixed Colors ===
#define COLOR_BG        0x0000
#define COLOR_TEXT       0xFFFF
#define COLOR_DIM        0x7BEF
#define COLOR_DIVIDER    0x4208
#define COLOR_BATTERY    0x07E0
#define COLOR_BAT_LOW    0xFD20
#define COLOR_BAT_CRIT   0xF800
#define COLOR_LIFE_WARN  0xFD20
#define COLOR_LIFE_CRIT  0xF800

// === MTG Mana Colors (RGB565) ===
#define MTG_WHITE   0xFFFF
#define MTG_BLUE    0x3B7F
#define MTG_BLACK   0x2945
#define MTG_RED     0xF800
#define MTG_GREEN   0x07E0

// === Color Themes ===

struct ColorTheme {
  uint16_t accent;
  uint16_t menuBg;
  uint16_t selBg;
  uint16_t selText;
  uint16_t title;
  uint16_t activeBar;
};

enum ThemeId {
  THEME_PLAINS,
  THEME_ISLAND,
  THEME_SWAMP,
  THEME_MOUNTAIN,
  THEME_FOREST,
  THEME_COUNT
};

const ColorTheme THEMES[] PROGMEM = {
  { 0xFEA0, 0x1082, 0x4A28, 0xFFFF, 0xFFFF, 0xFEA0 },  // Plains
  { 0x5DDF, 0x0849, 0x1293, 0xFFFF, 0x5DDF, 0x5DDF },  // Island
  { 0xC8DF, 0x1002, 0x3006, 0xFFFF, 0xC8DF, 0xC8DF },  // Swamp
  { 0xFBE0, 0x2000, 0x4800, 0xFFFF, 0xFBE0, 0xFBE0 },  // Mountain
  { 0x57EA, 0x0320, 0x0B40, 0xFFFF, 0x57EA, 0x57EA },  // Forest
};

// === States ===
enum AppState {
  STATE_MAIN_MENU,
  STATE_GAME_MODE_SELECT,
  STATE_CUSTOM_LIFE_INPUT,
  STATE_PLAYER_THEME_SELECT,
  STATE_GAME,
  STATE_DICE,
  STATE_COIN,
  STATE_CONFIRM_RESET,
  STATE_SETTINGS,
  STATE_ABOUT,
  STATE_DIAGNOSTICS,
  STATE_BATTERY_INFO,
  STATE_SYSTEM_INFO,
  STATE_GAME_STATS,
  STATE_TEMPERATURE,
  STATE_IMU_STATUS,
  STATE_TEST_MENU,
  STATE_IMU_CALIBRATION,
  STATE_BUTTON_TEST,
  STATE_SCREEN_TEST,
  STATE_SPEAKER_TEST,
  STATE_EASTER_EGGS_MENU,
  STATE_GAME_MANA_RUNNER,
  STATE_GAME_ARENA,
  STATE_GAME_SNAKE,
  STATE_GAME_SPELL_DODGE
};

enum MainMenuOption {
  MMENU_START_GAME,
  MMENU_OPTIONS,
  MMENU_ABOUT,
  MMENU_COUNT
};

enum GameModeOption {
  GMODE_STANDARD,
  GMODE_COMMANDER,
  GMODE_CUSTOM,
  GMODE_COUNT
};

enum GameMenuOption {
  GMENU_SWITCH_PLAYER,
  GMENU_DICE,
  GMENU_COIN,
  GMENU_SETTINGS,
  GMENU_RESET,
  GMENU_COUNT
};

enum SettingsOption {
  SET_BRIGHTNESS,
  SET_VOLUME,
  SET_THEME,
  SET_FACE_DOWN_PAUSE,
  SET_SHUTDOWN_IDLE,
  SET_SHUTDOWN_GAME,
  SET_DIAGNOSTICS,
  SET_BACK,
  SET_COUNT
};

enum TimerMode {
  TIMER_PER_TURN,
  TIMER_PER_MATCH,
  TIMER_MODE_COUNT
};

enum DiagnosticsOption {
  DIAG_BATTERY,
  DIAG_SYSTEM,
  DIAG_STATS,
  DIAG_TEMPERATURE,
  DIAG_IMU,
  DIAG_TESTS,
  DIAG_EASTER_EGGS,
  DIAG_BACK,
  DIAG_COUNT
};

enum EasterEggsOption {
  EE_MANA_RUNNER,
  EE_ARENA_BATTLE,
  EE_SNAKE,
  EE_SPELL_DODGE,
  EE_BACK,
  EE_COUNT
};

enum TestMenuOption {
  TEST_IMU_CALIBRATION,
  TEST_BUTTONS,
  TEST_SCREEN,
  TEST_SPEAKER,
  TEST_BACK,
  TEST_COUNT
};

#endif
