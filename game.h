#ifndef GAME_H
#define GAME_H

#include "config.h"
#include <Arduino.h>

struct HistoryEntry {
  int8_t playerIndex;
  int8_t delta;
};

struct Player {
  uint8_t life;
  ThemeId theme;
};

struct GameState {
  Player players[MAX_PLAYERS];
  HistoryEntry history[HISTORY_SIZE];
  uint8_t historyCount;
  uint8_t activePlayer;
  uint8_t startingLife;
  AppState appState;
  uint8_t menuSelection;
  bool gameOver;
  uint8_t loserIndex;
  uint8_t lastDiceResult;
  uint8_t diceType;
  bool lastCoinResult;
  bool showingResult;
  unsigned long turnStartMs;
  unsigned long matchStartMs;
  bool timerRunning;
};

void gameInit(GameState& gs, uint8_t startingLife);
void gameReset(GameState& gs);
void gameAddLife(GameState& gs, int8_t playerIndex, int8_t delta);
void gameCheckDefeat(GameState& gs);
uint8_t gameRollDice(GameState& gs, uint8_t sides);
bool gameFlipCoin(GameState& gs);
void gameSwitchPlayer(GameState& gs);
unsigned long gameGetMatchSeconds(const GameState& gs);

#endif
