#include "game.h"

void gameInit(GameState& gs, uint8_t startingLife) {
  gs.startingLife = startingLife;
  gs.activePlayer = 0;
  gs.historyCount = 0;
  gs.appState = STATE_GAME;
  gs.menuSelection = 0;
  gs.gameOver = false;
  gs.loserIndex = 0;
  gs.lastDiceResult = 0;
  gs.diceType = 20;
  gs.lastCoinResult = false;
  gs.showingResult = false;
  gs.turnStartMs = millis();
  gs.matchStartMs = millis();
  gs.timerRunning = true;

  for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
    gs.players[i].life = startingLife;
  }
  memset(gs.history, 0, sizeof(gs.history));
}

void gameReset(GameState& gs) {
  gameInit(gs, gs.startingLife);
}

static void pushHistory(GameState& gs, int8_t playerIndex, int8_t delta) {
  if (gs.historyCount >= HISTORY_SIZE) {
    memmove(&gs.history[0], &gs.history[1], sizeof(HistoryEntry) * (HISTORY_SIZE - 1));
    gs.historyCount = HISTORY_SIZE - 1;
  }
  gs.history[gs.historyCount].playerIndex = playerIndex;
  gs.history[gs.historyCount].delta = delta;
  gs.historyCount++;
}

void gameAddLife(GameState& gs, int8_t playerIndex, int8_t delta) {
  if (gs.gameOver) return;

  int16_t newLife = (int16_t)gs.players[playerIndex].life + delta;

  if (newLife > 255) {
    gs.players[playerIndex].life = 255;
  } else if (newLife < 0) {
    gs.players[playerIndex].life = 0;
  } else {
    gs.players[playerIndex].life = (uint8_t)newLife;
  }

  pushHistory(gs, playerIndex, delta);
  gameCheckDefeat(gs);
}

void gameCheckDefeat(GameState& gs) {
  for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
    if (gs.players[i].life == 0) {
      gs.gameOver = true;
      gs.loserIndex = i;
      gs.timerRunning = false;
      return;
    }
  }
}

uint8_t gameRollDice(GameState& gs, uint8_t sides) {
  gs.diceType = sides;
  gs.lastDiceResult = random(1, sides + 1);
  gs.showingResult = true;
  return gs.lastDiceResult;
}

bool gameFlipCoin(GameState& gs) {
  gs.lastCoinResult = random(0, 2) == 1;
  gs.showingResult = true;
  return gs.lastCoinResult;
}

void gameSwitchPlayer(GameState& gs) {
  gs.activePlayer = (gs.activePlayer + 1) % MAX_PLAYERS;
  gs.turnStartMs = millis();
}

unsigned long gameGetMatchSeconds(const GameState& gs) {
  if (!gs.timerRunning) return 0;
  return (millis() - gs.matchStartMs) / 1000;
}
