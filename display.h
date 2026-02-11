#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include "game.h"
#include <M5Unified.h>

M5Canvas& displayGetSprite();
void displayBeginDraw(uint16_t bg = COLOR_BG);
void displayEndDraw();
void displayDrawCentered(const char* text, int y, uint8_t size, uint16_t color, uint16_t bg = COLOR_BG);
const ColorTheme* displayGetTheme();

void displaySetTheme(ThemeId theme);
void displayInit();
void displayStartup();
void displayMainMenu(uint8_t selection);
void displayGameModeSelect(uint8_t selection);
void displayCustomLifeInput(uint8_t life);
void displayPlayerThemeSelect(uint8_t playerIdx, ThemeId selectedTheme);
void displayGame(const GameState& gs, TimerMode timerMode);
void displayGameMenu(const GameState& gs, uint8_t selection);
void displayDice(const GameState& gs);
void displayCoin(const GameState& gs);
void displayConfirmReset(uint8_t selection);
void displayGameOver(const GameState& gs, TimerMode timerMode);
void displaySettings(uint8_t selection, uint8_t brightness, uint8_t volume, TimerMode timerMode, ThemeId themeId, bool faceDownPause, uint8_t shutdownIdleIdx, uint8_t shutdownGameIdx);
void displayAbout();
void displayDiagnostics(uint8_t selection, bool hasEasterEggs);
void displayBatteryInfo();
void displaySystemInfo();
void displayGameStats(uint16_t totalMatches, uint32_t totalPlaytime,
                      uint16_t p1Wins, uint16_t p2Wins,
                      uint16_t diceRolls, uint16_t coinFlips);
void displayTemperature();
void displayIMUStatus();
void displayDiceAnimation(uint8_t sides);
void displayCoinAnimation();
void displayVictoryAnimation(uint8_t winnerIdx, const GameState& gs);
void displayTestMenu(uint8_t selection);
void displayIMUCalibration(bool inProgress, uint8_t samplesCollected, float magnitude);
void displayButtonTest(bool btnA, bool btnB, bool btnPWR);
void displayScreenTest(uint8_t pattern);
void displaySpeakerTest(uint16_t frequency);
void displayEasterEggsMenu(uint8_t selection);
void displayMiniGameOver(uint16_t score);

#endif
