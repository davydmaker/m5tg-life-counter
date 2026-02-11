#include <M5Unified.h>
#include <WiFi.h>
#include <esp_bt.h>
#include <Preferences.h>
#include "config.h"
#include "game.h"
#include "input.h"
#include "display.h"
#include "audio.h"
#include "joystick.h"
#include "minigames.h"

Preferences prefs;

GameState gameState;
InputState inputState;
unsigned long lastActivityMs = 0;
bool inGameMenu = false;
uint8_t gameMenuSel = 0;
bool imuAsleep = false;
AppState lastAppState = STATE_MAIN_MENU;

bool powerSavingActive = false;
unsigned long lastPowerCheckMs = 0;

bool isFaceDown = false;
unsigned long lastOrientationCheckMs = 0;
unsigned long faceDownStartMs = 0;
unsigned long pausedTimeMs = 0;

uint8_t settingBrightness = DEFAULT_BRIGHTNESS;
uint8_t settingVolume = SPEAKER_VOLUME;
TimerMode settingTimerMode = TIMER_PER_TURN;
ThemeId settingTheme = THEME_PLAINS;
bool settingFaceDownPause = true;
uint8_t settingShutdownIdleIdx = 0;
uint8_t settingShutdownGameIdx = 0;
uint8_t settingsSelection = 0;

uint8_t mainMenuSel = 0;
uint8_t gameModeSel = 0;
uint8_t diagnosticsSel = 0;

uint8_t customLifeInput = LIFE_DEFAULT;

uint8_t themeSelectPlayerIndex = 0;
ThemeId themeSelectChoice[MAX_PLAYERS] = { THEME_PLAINS, THEME_ISLAND };

uint16_t statTotalMatches = 0;
uint32_t statTotalPlaytimeSeconds = 0;
uint16_t statPlayer1Wins = 0;
uint16_t statPlayer2Wins = 0;
uint16_t statDiceRolls = 0;
uint16_t statCoinFlips = 0;

bool joystickConnected = false;
JoystickState joystickState;
uint8_t easterEggsSel = 0;

bool diagHasEasterEggs = false;

uint8_t testMenuSel = 0;
bool imuCalibrationInProgress = false;
uint8_t imuCalibrationSamples = 0;
uint8_t screenTestPattern = 0;
uint16_t speakerTestFrequency = 1000;

void resetActivity() {
  lastActivityMs = millis();
}

void imuSleep() {
  if (!imuAsleep) {
    M5.In_I2C.writeRegister8(0x68, 0x6B, 0x40, 100000);
    imuAsleep = true;
  }
}

void imuWake() {
  if (imuAsleep) {
    M5.In_I2C.writeRegister8(0x68, 0x6B, 0x00, 100000);
    delay(10);
    imuAsleep = false;
  }
}

bool imuShouldBeAwake(AppState state) {
  return (state == STATE_GAME || state == STATE_DICE || state == STATE_COIN || state == STATE_IMU_STATUS || state == STATE_IMU_CALIBRATION);
}

void checkPowerSaving() {
  int8_t batteryLevel = M5.Power.getBatteryLevel();

  if (batteryLevel <= POWER_SAVE_BATTERY_THRESHOLD && !powerSavingActive) {
    powerSavingActive = true;
    setCpuFrequencyMhz(POWER_SAVE_CPU_MHZ);
    M5.Display.setBrightness(POWER_SAVE_BRIGHTNESS);
  } else if (batteryLevel > POWER_SAVE_BATTERY_THRESHOLD && powerSavingActive) {
    powerSavingActive = false;
    setCpuFrequencyMhz(240);
    M5.Display.setBrightness(settingBrightness);
  }
}

void checkFaceDown() {
  if (!settingFaceDownPause || gameState.appState != STATE_GAME || gameState.gameOver) {
    return;
  }

  float accX, accY, accZ;
  M5.Imu.getAccelData(&accX, &accY, &accZ);

  bool nowFaceDown = (accZ < FACE_DOWN_THRESHOLD);

  if (nowFaceDown && !isFaceDown) {
    isFaceDown = true;
    faceDownStartMs = millis();
    M5.Display.setBrightness(0);
    M5.Display.sleep();
  } else if (!nowFaceDown && isFaceDown) {
    isFaceDown = false;
    unsigned long pauseDuration = millis() - faceDownStartMs;
    pausedTimeMs += pauseDuration;

    if (gameState.timerRunning) {
      gameState.turnStartMs += pauseDuration;
      gameState.matchStartMs += pauseDuration;
    }

    M5.Display.wakeup();
    if (powerSavingActive) {
      M5.Display.setBrightness(POWER_SAVE_BRIGHTNESS);
    } else {
      M5.Display.setBrightness(settingBrightness);
    }
    displayGame(gameState, settingTimerMode);
  }
}

void loadConfig() {
  prefs.begin("mtg-config", true);
  settingBrightness = prefs.getUChar("brightness", DEFAULT_BRIGHTNESS);
  settingVolume = prefs.getUChar("volume", SPEAKER_VOLUME);
  settingTimerMode = (TimerMode)prefs.getUChar("timerMode", TIMER_PER_TURN);
  settingTheme = (ThemeId)prefs.getUChar("theme", THEME_PLAINS);
  settingFaceDownPause = prefs.getBool("faceDownPause", true);
  settingShutdownIdleIdx = prefs.getUChar("shutIdleIdx", 0);
  settingShutdownGameIdx = prefs.getUChar("shutGameIdx", 0);

  statTotalMatches = prefs.getUShort("totalMatches", 0);
  statTotalPlaytimeSeconds = prefs.getUInt("totalPlaytime", 0);
  statPlayer1Wins = prefs.getUShort("p1wins", 0);
  statPlayer2Wins = prefs.getUShort("p2wins", 0);
  statDiceRolls = prefs.getUShort("diceRolls", 0);
  statCoinFlips = prefs.getUShort("coinFlips", 0);

  prefs.end();
}

void saveConfig() {
  prefs.begin("mtg-config", false);
  prefs.putUChar("brightness", settingBrightness);
  prefs.putUChar("volume", settingVolume);
  prefs.putUChar("timerMode", (uint8_t)settingTimerMode);
  prefs.putUChar("theme", (uint8_t)settingTheme);
  prefs.putBool("faceDownPause", settingFaceDownPause);
  prefs.putUChar("shutIdleIdx", settingShutdownIdleIdx);
  prefs.putUChar("shutGameIdx", settingShutdownGameIdx);
  prefs.end();
}

void saveStats() {
  prefs.begin("mtg-config", false);
  prefs.putUShort("totalMatches", statTotalMatches);
  prefs.putUInt("totalPlaytime", statTotalPlaytimeSeconds);
  prefs.putUShort("p1wins", statPlayer1Wins);
  prefs.putUShort("p2wins", statPlayer2Wins);
  prefs.putUShort("diceRolls", statDiceRolls);
  prefs.putUShort("coinFlips", statCoinFlips);
  prefs.end();
}

void resetStats() {
  statTotalMatches = 0;
  statTotalPlaytimeSeconds = 0;
  statPlayer1Wins = 0;
  statPlayer2Wins = 0;
  statDiceRolls = 0;
  statCoinFlips = 0;

  saveStats();
}

void handleMainMenu(InputEvent evt) {
  switch (evt) {
    case INPUT_B_PRESS:
      mainMenuSel = (mainMenuSel + 1) % MMENU_COUNT;
      displayMainMenu(mainMenuSel);
      break;
    case INPUT_A_PRESS:
      audioConfirm();
      switch ((MainMenuOption)mainMenuSel) {
        case MMENU_START_GAME:
          gameState.appState = STATE_GAME_MODE_SELECT;
          gameModeSel = 0;
          displayGameModeSelect(gameModeSel);
          break;
        case MMENU_OPTIONS:
          gameState.appState = STATE_SETTINGS;
          settingsSelection = 0;
          redrawSettings();
          break;
        case MMENU_ABOUT:
          gameState.appState = STATE_ABOUT;
          displayAbout();
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void handleGameModeSelect(InputEvent evt) {
  switch (evt) {
    case INPUT_B_PRESS:
      gameModeSel = (gameModeSel + 1) % GMODE_COUNT;
      displayGameModeSelect(gameModeSel);
      break;
    case INPUT_A_PRESS:
      {
        audioConfirm();
        if (gameModeSel == GMODE_CUSTOM) {
          gameState.appState = STATE_CUSTOM_LIFE_INPUT;
          customLifeInput = LIFE_DEFAULT;
          displayCustomLifeInput(customLifeInput);
        } else {
          customLifeInput = (gameModeSel == GMODE_STANDARD) ? LIFE_STANDARD : LIFE_COMMANDER;
          gameState.appState = STATE_PLAYER_THEME_SELECT;
          themeSelectPlayerIndex = 0;
          displayPlayerThemeSelect(themeSelectPlayerIndex, themeSelectChoice[themeSelectPlayerIndex]);
        }
        break;
      }
    case INPUT_PWR:
      gameState.appState = STATE_MAIN_MENU;
      mainMenuSel = 0;
      displayMainMenu(mainMenuSel);
      break;
    default:
      break;
  }
}

void handleCustomLifeInput(InputEvent evt) {
  switch (evt) {
    case INPUT_A_PRESS:
      if (customLifeInput < LIFE_MAX) {
        customLifeInput++;
        displayCustomLifeInput(customLifeInput);
      }
      break;

    case INPUT_B_PRESS:
      if (customLifeInput > LIFE_MIN) {
        customLifeInput--;
        displayCustomLifeInput(customLifeInput);
      }
      break;

    case INPUT_A_LONG:
      customLifeInput = (customLifeInput + 5 > LIFE_MAX) ? LIFE_MAX : customLifeInput + 5;
      displayCustomLifeInput(customLifeInput);
      break;

    case INPUT_B_LONG:
      customLifeInput = (customLifeInput - 5 < LIFE_MIN) ? LIFE_MIN : customLifeInput - 5;
      displayCustomLifeInput(customLifeInput);
      break;

    case INPUT_PWR:
      audioConfirm();
      gameState.appState = STATE_PLAYER_THEME_SELECT;
      themeSelectPlayerIndex = 0;
      displayPlayerThemeSelect(themeSelectPlayerIndex, themeSelectChoice[themeSelectPlayerIndex]);
      break;

    default:
      break;
  }
}

void handlePlayerThemeSelect(InputEvent evt) {
  switch (evt) {
    case INPUT_B_PRESS:
      themeSelectChoice[themeSelectPlayerIndex] = (ThemeId)((themeSelectChoice[themeSelectPlayerIndex] + 1) % THEME_COUNT);
      displayPlayerThemeSelect(themeSelectPlayerIndex, themeSelectChoice[themeSelectPlayerIndex]);
      break;

    case INPUT_A_PRESS:
      audioConfirm();
      if (themeSelectPlayerIndex == 0) {
        themeSelectPlayerIndex = 1;
        displayPlayerThemeSelect(themeSelectPlayerIndex, themeSelectChoice[themeSelectPlayerIndex]);
      } else {
        gameInit(gameState, customLifeInput);
        gameState.players[0].theme = themeSelectChoice[0];
        gameState.players[1].theme = themeSelectChoice[1];
        displayGame(gameState, settingTimerMode);
      }
      break;

    default:
      break;
  }
}

void handleGameMenu(InputEvent evt) {
  switch (evt) {
    case INPUT_B_PRESS:
      gameMenuSel = (gameMenuSel + 1) % GMENU_COUNT;
      displayGameMenu(gameState, gameMenuSel);
      break;
    case INPUT_A_PRESS:
      {
        switch ((GameMenuOption)gameMenuSel) {
          case GMENU_SWITCH_PLAYER:
            gameSwitchPlayer(gameState);
            audioConfirm();
            inGameMenu = false;
            gameState.appState = STATE_GAME;
            displayGame(gameState, settingTimerMode);
            break;
          case GMENU_DICE:
            inGameMenu = false;
            gameState.appState = STATE_DICE;
            gameState.showingResult = false;
            displayDice(gameState);
            break;
          case GMENU_COIN:
            inGameMenu = false;
            gameState.appState = STATE_COIN;
            gameState.showingResult = false;
            displayCoin(gameState);
            break;
          case GMENU_SETTINGS:
            inGameMenu = false;
            gameState.appState = STATE_SETTINGS;
            settingsSelection = 0;
            redrawSettings();
            break;
          case GMENU_RESET:
            inGameMenu = false;
            gameState.appState = STATE_CONFIRM_RESET;
            gameState.menuSelection = 0;
            displayConfirmReset(0);
            break;
          default:
            break;
        }
        break;
      }
    case INPUT_PWR:
      inGameMenu = false;
      gameState.appState = STATE_GAME;
      displayGame(gameState, settingTimerMode);
      break;
    default:
      break;
  }
}

void applyLifeChange(int8_t delta) {
  gameAddLife(gameState, gameState.activePlayer, delta);
  if (delta > 0) audioLifeUp(); else audioLifeDown();
  if (gameState.gameOver) {
    statTotalMatches++;
    statTotalPlaytimeSeconds += gameGetMatchSeconds(gameState);
    uint8_t winner = (gameState.loserIndex == 0) ? 1 : 0;
    if (winner == 0) statPlayer1Wins++; else statPlayer2Wins++;
    saveStats();
    displayVictoryAnimation(winner, gameState);
    audioVictory();
    displayGameOver(gameState, settingTimerMode);
  } else {
    displayGame(gameState, settingTimerMode);
  }
}

void handleGame(InputEvent evt) {
  if (gameState.gameOver) {
    if (evt == INPUT_A_PRESS) {
      gameState.appState = STATE_GAME_MODE_SELECT;
      gameModeSel = 0;
      displayGameModeSelect(gameModeSel);
    } else if (evt == INPUT_PWR) {
      gameState.appState = STATE_MAIN_MENU;
      mainMenuSel = 0;
      displayMainMenu(mainMenuSel);
    }
    return;
  }

  switch (evt) {
    case INPUT_A_PRESS:  applyLifeChange(1);  break;
    case INPUT_B_PRESS:  applyLifeChange(-1); break;
    case INPUT_A_LONG:   applyLifeChange(5);  break;
    case INPUT_B_LONG:   applyLifeChange(-5); break;

    case INPUT_PWR:
      inGameMenu = true;
      gameMenuSel = 0;
      displayGameMenu(gameState, gameMenuSel);
      break;

    case INPUT_SHAKE:
      gameSwitchPlayer(gameState);
      audioConfirm();
      displayGame(gameState, settingTimerMode);
      break;

    default:
      break;
  }
}

void handleDice(InputEvent evt) {
  switch (evt) {
    case INPUT_A_PRESS:
      {
        uint8_t diceOptions[] = { 4, 6, 8, 10, 12, 20, 100 };
        uint8_t optionCount = 7;

        uint8_t currentIdx = 0;
        for (uint8_t i = 0; i < optionCount; i++) {
          if (gameState.diceType == diceOptions[i]) {
            currentIdx = i;
            break;
          }
        }

        gameState.diceType = diceOptions[(currentIdx + 1) % optionCount];
        gameState.showingResult = false;
        displayDice(gameState);
        break;
      }
    case INPUT_B_PRESS:
    case INPUT_SHAKE:
      displayDiceAnimation(gameState.diceType);
      audioDiceRoll();
      gameRollDice(gameState, gameState.diceType);
      statDiceRolls++;
      saveStats();
      displayDice(gameState);
      break;
    case INPUT_PWR:
      gameState.appState = STATE_GAME;
      gameState.showingResult = false;
      displayGame(gameState, settingTimerMode);
      break;
    default:
      break;
  }
}

void handleCoin(InputEvent evt) {
  switch (evt) {
    case INPUT_B_PRESS:
    case INPUT_SHAKE:
      displayCoinAnimation();
      audioCoinFlip();
      gameFlipCoin(gameState);
      statCoinFlips++;
      saveStats();
      displayCoin(gameState);
      break;
    case INPUT_PWR:
      gameState.appState = STATE_GAME;
      gameState.showingResult = false;
      displayGame(gameState, settingTimerMode);
      break;
    default:
      break;
  }
}

void redrawSettings() {
  displaySettings(settingsSelection, settingBrightness, settingVolume,
                  settingTimerMode, settingTheme, settingFaceDownPause,
                  settingShutdownIdleIdx, settingShutdownGameIdx);
}

void redrawDiagnostics() {
  displayDiagnostics(diagnosticsSel, diagHasEasterEggs);
}

void handleSettings(InputEvent evt) {
  bool fromGame = (gameState.timerRunning && !gameState.gameOver);

  switch (evt) {
    case INPUT_B_PRESS:
      settingsSelection = (settingsSelection + 1) % SET_COUNT;
      redrawSettings();
      break;

    case INPUT_A_PRESS:
      if (settingsSelection == SET_BRIGHTNESS) {
        settingBrightness = (settingBrightness >= 240) ? 25 : settingBrightness + 25;
        M5.Display.setBrightness(settingBrightness);
        saveConfig();
        redrawSettings();
      } else if (settingsSelection == SET_VOLUME) {
        settingVolume = (settingVolume >= 240) ? 0 : settingVolume + 30;
        M5.Speaker.setVolume(settingVolume);
        saveConfig();
        audioConfirm();
        redrawSettings();
      } else if (settingsSelection == SET_THEME) {
        settingTheme = (ThemeId)((settingTheme + 1) % THEME_COUNT);
        displaySetTheme(settingTheme);
        saveConfig();
        audioConfirm();
        redrawSettings();
      } else if (settingsSelection == SET_FACE_DOWN_PAUSE) {
        settingFaceDownPause = !settingFaceDownPause;
        saveConfig();
        audioConfirm();
        redrawSettings();
      } else if (settingsSelection == SET_SHUTDOWN_IDLE) {
        settingShutdownIdleIdx = (settingShutdownIdleIdx + 1) % SHUTDOWN_IDLE_COUNT;
        saveConfig();
        audioConfirm();
        redrawSettings();
      } else if (settingsSelection == SET_SHUTDOWN_GAME) {
        settingShutdownGameIdx = (settingShutdownGameIdx + 1) % SHUTDOWN_GAME_COUNT;
        saveConfig();
        audioConfirm();
        redrawSettings();
      } else if (settingsSelection == SET_DIAGNOSTICS) {
        audioConfirm();
        gameState.appState = STATE_DIAGNOSTICS;
        diagnosticsSel = 0;
        detectEasterEggsHAT();
        redrawDiagnostics();
      } else if (settingsSelection == SET_BACK) {
        if (fromGame) {
          gameState.appState = STATE_GAME;
          displayGame(gameState, settingTimerMode);
        } else {
          gameState.appState = STATE_MAIN_MENU;
          displayMainMenu(mainMenuSel);
        }
      }
      break;

    case INPUT_A_LONG:
      if (settingsSelection == SET_BRIGHTNESS) {
        settingBrightness = (settingBrightness >= 240) ? 255 : settingBrightness + 25;
        M5.Display.setBrightness(settingBrightness);
        redrawSettings();
      } else if (settingsSelection == SET_VOLUME) {
        settingVolume = (settingVolume >= 240) ? 255 : settingVolume + 30;
        M5.Speaker.setVolume(settingVolume);
        redrawSettings();
      } else if (settingsSelection == SET_BACK && fromGame) {
        gameState.appState = STATE_MAIN_MENU;
        mainMenuSel = 0;
        displayMainMenu(mainMenuSel);
      }
      break;

    case INPUT_B_LONG:
      if (settingsSelection == SET_BRIGHTNESS) {
        settingBrightness = (settingBrightness <= 25) ? 5 : settingBrightness - 25;
        M5.Display.setBrightness(settingBrightness);
        redrawSettings();
      } else if (settingsSelection == SET_VOLUME) {
        settingVolume = (settingVolume <= 30) ? 0 : settingVolume - 30;
        M5.Speaker.setVolume(settingVolume);
        redrawSettings();
      }
      break;

    case INPUT_PWR:
      if (fromGame) {
        gameState.appState = STATE_GAME;
        displayGame(gameState, settingTimerMode);
      } else {
        gameState.appState = STATE_MAIN_MENU;
        displayMainMenu(mainMenuSel);
      }
      break;

    default:
      break;
  }
}

void handleAbout(InputEvent evt) {
  if (evt == INPUT_PWR || evt == INPUT_A_PRESS || evt == INPUT_B_PRESS) {
    gameState.appState = STATE_MAIN_MENU;
    mainMenuSel = 0;
    displayMainMenu(mainMenuSel);
  }
}

void detectEasterEggsHAT() {
  joystickConnected = false;
  joystickInit();
  joystickConnected = joystickDetect();
  diagHasEasterEggs = joystickConnected;
}

void handleDiagnostics(InputEvent evt) {
  switch (evt) {
    case INPUT_B_PRESS:
      diagnosticsSel = (diagnosticsSel + 1) % DIAG_COUNT;
      if (diagnosticsSel == DIAG_EASTER_EGGS && !diagHasEasterEggs) {
        diagnosticsSel = (diagnosticsSel + 1) % DIAG_COUNT;
      }
      redrawDiagnostics();
      break;
    case INPUT_A_PRESS:
      audioConfirm();
      switch ((DiagnosticsOption)diagnosticsSel) {
        case DIAG_BATTERY:
          gameState.appState = STATE_BATTERY_INFO;
          displayBatteryInfo();
          break;
        case DIAG_SYSTEM:
          gameState.appState = STATE_SYSTEM_INFO;
          displaySystemInfo();
          break;
        case DIAG_STATS:
          gameState.appState = STATE_GAME_STATS;
          displayGameStats(statTotalMatches, statTotalPlaytimeSeconds,
                           statPlayer1Wins, statPlayer2Wins,
                           statDiceRolls, statCoinFlips);
          break;
        case DIAG_TEMPERATURE:
          gameState.appState = STATE_TEMPERATURE;
          displayTemperature();
          break;
        case DIAG_IMU:
          gameState.appState = STATE_IMU_STATUS;
          displayIMUStatus();
          break;
        case DIAG_TESTS:
          gameState.appState = STATE_TEST_MENU;
          testMenuSel = 0;
          displayTestMenu(testMenuSel);
          break;
        case DIAG_EASTER_EGGS:
          if (joystickConnected) {
            gameState.appState = STATE_EASTER_EGGS_MENU;
            easterEggsSel = 0;
            displayEasterEggsMenu(easterEggsSel);
          }
          break;
        case DIAG_BACK:
          gameState.appState = STATE_SETTINGS;
          redrawSettings();
          break;
      }
      break;
    case INPUT_PWR:
      gameState.appState = STATE_SETTINGS;
      redrawSettings();
      break;
    default:
      break;
  }
}

void handleBatteryInfo(InputEvent evt) {
  if (evt == INPUT_PWR || evt == INPUT_A_PRESS || evt == INPUT_B_PRESS) {
    gameState.appState = STATE_DIAGNOSTICS;
    redrawDiagnostics();
  }
}

void handleSystemInfo(InputEvent evt) {
  if (evt == INPUT_PWR || evt == INPUT_A_PRESS || evt == INPUT_B_PRESS) {
    gameState.appState = STATE_DIAGNOSTICS;
    redrawDiagnostics();
  }
}

void handleGameStats(InputEvent evt) {
  if (evt == INPUT_A_LONG) {
    resetStats();
    audioDefeat();
    displayGameStats(statTotalMatches, statTotalPlaytimeSeconds,
                     statPlayer1Wins, statPlayer2Wins,
                     statDiceRolls, statCoinFlips);
  } else if (evt == INPUT_PWR || evt == INPUT_B_PRESS) {
    gameState.appState = STATE_DIAGNOSTICS;
    redrawDiagnostics();
  }
}

void handleTemperature(InputEvent evt) {
  if (evt == INPUT_PWR || evt == INPUT_A_PRESS || evt == INPUT_B_PRESS) {
    gameState.appState = STATE_DIAGNOSTICS;
    redrawDiagnostics();
  }
}

void handleIMUStatus(InputEvent evt) {
  if (evt == INPUT_PWR || evt == INPUT_A_PRESS || evt == INPUT_B_PRESS) {
    gameState.appState = STATE_DIAGNOSTICS;
    redrawDiagnostics();
  }
}

void handleTestMenu(InputEvent evt) {
  switch (evt) {
    case INPUT_B_PRESS:
      testMenuSel = (testMenuSel + 1) % TEST_COUNT;
      displayTestMenu(testMenuSel);
      break;
    case INPUT_A_PRESS:
      audioConfirm();
      switch ((TestMenuOption)testMenuSel) {
        case TEST_IMU_CALIBRATION:
          {
            gameState.appState = STATE_IMU_CALIBRATION;
            imuCalibrationInProgress = false;
            imuCalibrationSamples = 0;
            float accX, accY, accZ;
            M5.Imu.getAccelData(&accX, &accY, &accZ);
            float mag = sqrt(accX * accX + accY * accY + accZ * accZ);
            displayIMUCalibration(false, 0, mag);
            break;
          }
        case TEST_BUTTONS:
          gameState.appState = STATE_BUTTON_TEST;
          displayButtonTest(false, false, false);
          break;
        case TEST_SCREEN:
          gameState.appState = STATE_SCREEN_TEST;
          screenTestPattern = 0;
          displayScreenTest(screenTestPattern);
          break;
        case TEST_SPEAKER:
          gameState.appState = STATE_SPEAKER_TEST;
          speakerTestFrequency = 1000;
          M5.Speaker.tone(speakerTestFrequency, 100);
          displaySpeakerTest(speakerTestFrequency);
          break;
        case TEST_BACK:
          gameState.appState = STATE_DIAGNOSTICS;
          redrawDiagnostics();
          break;
      }
      break;
    case INPUT_PWR:
      gameState.appState = STATE_DIAGNOSTICS;
      redrawDiagnostics();
      break;
    default:
      break;
  }
}

void handleIMUCalibration(InputEvent evt) {
  static float sumMag = 0;

  if (evt == INPUT_A_PRESS && !imuCalibrationInProgress) {
    audioConfirm();
    imuCalibrationInProgress = true;
    imuCalibrationSamples = 0;
    sumMag = 0;
    displayIMUCalibration(true, 0, 0);
  } else if (evt == INPUT_PWR) {
    gameState.appState = STATE_TEST_MENU;
    displayTestMenu(testMenuSel);
  }

  if (imuCalibrationInProgress) {
    static unsigned long lastSample = 0;
    if (millis() - lastSample > 100) {
      lastSample = millis();
      float accX, accY, accZ;
      M5.Imu.getAccelData(&accX, &accY, &accZ);
      float mag = sqrt(accX * accX + accY * accY + accZ * accZ);

      sumMag += mag;
      imuCalibrationSamples++;

      displayIMUCalibration(true, imuCalibrationSamples, mag);

      if (imuCalibrationSamples >= 20) {
        float newBaseline = sumMag / 20.0f;
        inputState.baselineMag = newBaseline;

        audioConfirm();
        imuCalibrationInProgress = false;
        sumMag = 0;
        displayIMUCalibration(false, 0, newBaseline);
      }
    }
  }
}

void handleButtonTest(InputEvent evt) {
  static unsigned long lastUpdate = 0;
  static unsigned long pwrPressStart = 0;

  if (M5.BtnPWR.isPressed()) {
    if (pwrPressStart == 0) {
      pwrPressStart = millis();
    } else if (millis() - pwrPressStart > LONG_PRESS_MS) {
      gameState.appState = STATE_TEST_MENU;
      displayTestMenu(testMenuSel);
      pwrPressStart = 0;
      return;
    }
  } else {
    pwrPressStart = 0;
  }

  if (millis() - lastUpdate > 50) {
    lastUpdate = millis();
    bool btnA = M5.BtnA.isPressed();
    bool btnB = M5.BtnB.isPressed();
    bool btnPWR = M5.BtnPWR.isPressed();
    displayButtonTest(btnA, btnB, btnPWR);
  }
}

void handleScreenTest(InputEvent evt) {
  switch (evt) {
    case INPUT_B_PRESS:
      screenTestPattern = (screenTestPattern + 1) % 6;
      displayScreenTest(screenTestPattern);
      break;
    case INPUT_PWR:
      gameState.appState = STATE_TEST_MENU;
      displayTestMenu(testMenuSel);
      break;
    default:
      break;
  }
}

void handleSpeakerTest(InputEvent evt) {
  switch (evt) {
    case INPUT_A_PRESS:
      speakerTestFrequency += 100;
      if (speakerTestFrequency > 2000) speakerTestFrequency = 2000;
      M5.Speaker.tone(speakerTestFrequency, 100);
      displaySpeakerTest(speakerTestFrequency);
      break;
    case INPUT_B_PRESS:
      speakerTestFrequency -= 100;
      if (speakerTestFrequency < 100) speakerTestFrequency = 100;
      M5.Speaker.tone(speakerTestFrequency, 100);
      displaySpeakerTest(speakerTestFrequency);
      break;
    case INPUT_PWR:
      gameState.appState = STATE_TEST_MENU;
      displayTestMenu(testMenuSel);
      break;
    default:
      break;
  }
}

void handleEasterEggsMenu(InputEvent evt) {
  switch (evt) {
    case INPUT_B_PRESS:
      easterEggsSel = (easterEggsSel + 1) % EE_COUNT;
      displayEasterEggsMenu(easterEggsSel);
      break;
    case INPUT_A_PRESS:
      audioConfirm();
      switch ((EasterEggsOption)easterEggsSel) {
        case EE_MANA_RUNNER:
          gameState.appState = STATE_GAME_MANA_RUNNER;
          mgInit(STATE_GAME_MANA_RUNNER);
          break;
        case EE_ARENA_BATTLE:
          gameState.appState = STATE_GAME_ARENA;
          mgInit(STATE_GAME_ARENA);
          break;
        case EE_SNAKE:
          gameState.appState = STATE_GAME_SNAKE;
          mgInit(STATE_GAME_SNAKE);
          break;
        case EE_SPELL_DODGE:
          gameState.appState = STATE_GAME_SPELL_DODGE;
          mgInit(STATE_GAME_SPELL_DODGE);
          break;
        case EE_BACK:
          gameState.appState = STATE_DIAGNOSTICS;
          redrawDiagnostics();
          break;
        default:
          break;
      }
      break;
    case INPUT_PWR:
      gameState.appState = STATE_DIAGNOSTICS;
      redrawDiagnostics();
      break;
    default:
      break;
  }
}

void handleMinigame(InputEvent evt) {
  if (!mgIsAlive(gameState.appState)) {
    if (evt == INPUT_A_PRESS || evt == INPUT_B_PRESS || evt == INPUT_PWR) {
      gameState.appState = STATE_EASTER_EGGS_MENU;
      displayEasterEggsMenu(easterEggsSel);
    }
    return;
  }
  if (evt == INPUT_PWR) {
    gameState.appState = STATE_EASTER_EGGS_MENU;
    displayEasterEggsMenu(easterEggsSel);
  }
}

void handleConfirmReset(InputEvent evt) {
  switch (evt) {
    case INPUT_B_PRESS:
      gameState.menuSelection = (gameState.menuSelection + 1) % 2;
      displayConfirmReset(gameState.menuSelection);
      break;
    case INPUT_A_PRESS:
      if (gameState.menuSelection == 1) {
        gameReset(gameState);
        audioConfirm();
        displayGame(gameState, settingTimerMode);
      } else {
        gameState.appState = STATE_GAME;
        displayGame(gameState, settingTimerMode);
      }
      break;
    case INPUT_PWR:
      gameState.appState = STATE_GAME;
      displayGame(gameState, settingTimerMode);
      break;
    default:
      break;
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  WiFi.mode(WIFI_OFF);
  btStop();

  loadConfig();

  displayInit();
  audioInit();
  inputInit(inputState);

  M5.Display.setBrightness(settingBrightness);
  M5.Speaker.setVolume(settingVolume);
  displaySetTheme(settingTheme);

  randomSeed(analogRead(0) ^ micros());

  joystickInit();

  displayStartup();
  audioStartup();
  delay(1500);

  gameState.appState = STATE_MAIN_MENU;
  mainMenuSel = 0;
  displayMainMenu(mainMenuSel);

  lastActivityMs = millis();

  imuSleep();
}

bool shouldRefresh(unsigned long &lastTime, unsigned long interval) {
  if (millis() - lastTime >= interval) {
    lastTime = millis();
    return true;
  }
  return false;
}

void loop() {
  M5.update();

  InputEvent evt = inputUpdate(inputState);

  if (evt != INPUT_NONE) {
    resetActivity();
  }

  if (inGameMenu) {
    handleGameMenu(evt);
  } else {
    switch (gameState.appState) {
      case STATE_MAIN_MENU: handleMainMenu(evt); break;
      case STATE_GAME_MODE_SELECT: handleGameModeSelect(evt); break;
      case STATE_CUSTOM_LIFE_INPUT: handleCustomLifeInput(evt); break;
      case STATE_PLAYER_THEME_SELECT: handlePlayerThemeSelect(evt); break;
      case STATE_GAME: handleGame(evt); break;
      case STATE_DICE: handleDice(evt); break;
      case STATE_COIN: handleCoin(evt); break;
      case STATE_SETTINGS: handleSettings(evt); break;
      case STATE_ABOUT: handleAbout(evt); break;
      case STATE_DIAGNOSTICS: handleDiagnostics(evt); break;
      case STATE_BATTERY_INFO: handleBatteryInfo(evt); break;
      case STATE_SYSTEM_INFO: handleSystemInfo(evt); break;
      case STATE_GAME_STATS: handleGameStats(evt); break;
      case STATE_TEMPERATURE: handleTemperature(evt); break;
      case STATE_IMU_STATUS: handleIMUStatus(evt); break;
      case STATE_TEST_MENU: handleTestMenu(evt); break;
      case STATE_IMU_CALIBRATION: handleIMUCalibration(evt); break;
      case STATE_BUTTON_TEST: handleButtonTest(evt); break;
      case STATE_SCREEN_TEST: handleScreenTest(evt); break;
      case STATE_SPEAKER_TEST: handleSpeakerTest(evt); break;
      case STATE_CONFIRM_RESET: handleConfirmReset(evt); break;
      case STATE_EASTER_EGGS_MENU: handleEasterEggsMenu(evt); break;
      case STATE_GAME_MANA_RUNNER:
      case STATE_GAME_ARENA:
      case STATE_GAME_SNAKE:
      case STATE_GAME_SPELL_DODGE: handleMinigame(evt); break;
    }
  }

  static unsigned long lastRefresh = 0;
  if (gameState.appState == STATE_GAME && !inGameMenu && !gameState.gameOver) {
    if (millis() - lastRefresh > 1000) {
      lastRefresh = millis();
      displayGame(gameState, settingTimerMode);
    }
  }

  static unsigned long lastBatteryRefresh = 0;
  static unsigned long lastSystemRefresh = 0;
  static unsigned long lastTempRefresh = 0;
  static unsigned long lastIMURefresh = 0;

  if (gameState.appState == STATE_BATTERY_INFO && shouldRefresh(lastBatteryRefresh, 500))
    displayBatteryInfo();
  if (gameState.appState == STATE_SYSTEM_INFO && shouldRefresh(lastSystemRefresh, 1000))
    displaySystemInfo();
  if (gameState.appState == STATE_TEMPERATURE && shouldRefresh(lastTempRefresh, 1000))
    displayTemperature();
  if (gameState.appState == STATE_IMU_STATUS && shouldRefresh(lastIMURefresh, 100))
    displayIMUStatus();

  if (gameState.appState != lastAppState) {
    bool shouldBeAwake = imuShouldBeAwake(gameState.appState);
    if (shouldBeAwake && imuAsleep) {
      imuWake();
    } else if (!shouldBeAwake && !imuAsleep) {
      imuSleep();
    }
    lastAppState = gameState.appState;
  }

  if (millis() - lastPowerCheckMs > POWER_CHECK_INTERVAL_MS) {
    lastPowerCheckMs = millis();
    checkPowerSaving();
  }

  if (millis() - lastOrientationCheckMs > ORIENTATION_CHECK_INTERVAL_MS) {
    lastOrientationCheckMs = millis();
    checkFaceDown();
  }

  static unsigned long lastGameFrame = 0;
  if (gameState.appState >= STATE_GAME_MANA_RUNNER && gameState.appState <= STATE_GAME_SPELL_DODGE) {
    if (millis() - lastGameFrame >= 50) {
      lastGameFrame = millis();
      joystickRead(joystickState);
      if (mgIsAlive(gameState.appState)) {
        mgUpdate(gameState.appState, joystickState);
      }
      mgRender(gameState.appState);
    }
  }

  unsigned long shutdownTimeout = gameState.timerRunning
                                    ? pgm_read_dword(&SHUTDOWN_GAME_MS[settingShutdownGameIdx])
                                    : pgm_read_dword(&SHUTDOWN_IDLE_MS[settingShutdownIdleIdx]);

  if (millis() - lastActivityMs > shutdownTimeout) {
    M5.Display.fillScreen(COLOR_BG);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(COLOR_DIM, COLOR_BG);
    M5.Display.setCursor(40, 55);
    M5.Display.print("Powering off...");
    delay(1000);
    M5.Power.powerOff();
  }

  delay(10);
}
