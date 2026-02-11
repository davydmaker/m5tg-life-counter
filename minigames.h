#ifndef MINIGAMES_H
#define MINIGAMES_H

#include "config.h"
#include "joystick.h"

#define MR_MAX_OBSTACLES 6
#define MR_MAX_MANA 4
#define MR_PLAYER_SIZE 8
#define MR_PLAY_Y 14
#define MR_PLAY_H 121

struct MRObstacle {
  float x;
  uint8_t gapY;
  uint8_t gapSize;
  bool active;
};

struct MRMana {
  float x, y;
  uint8_t colorIdx;
  bool active;
};

struct ManaRunnerState {
  float playerY;
  MRObstacle obstacles[MR_MAX_OBSTACLES];
  MRMana mana[MR_MAX_MANA];
  uint16_t score;
  float speed;
  bool alive;
  unsigned long lastSpawnMs;
  unsigned long startMs;
};

#define AB_MAX_ENEMIES 10
#define AB_PLAY_Y 14
#define AB_PLAY_H 121

struct ABEnemy {
  float x, y;
  float dx, dy;
  uint8_t colorIdx;
  bool alive;
};

struct ArenaBattleState {
  float playerX, playerY;
  ABEnemy enemies[AB_MAX_ENEMIES];
  uint8_t hp;
  uint16_t score;
  uint8_t wave;
  bool attacking;
  unsigned long attackStartMs;
  unsigned long lastSpawnMs;
  unsigned long lastHitMs;
  bool alive;
};

#define SNAKE_CELL 10
#define SNAKE_COLS 24
#define SNAKE_ROWS 12
#define SNAKE_MAX_LEN 80
#define SNAKE_OFFSET_Y 14

struct SnakeState {
  uint8_t bodyX[SNAKE_MAX_LEN];
  uint8_t bodyY[SNAKE_MAX_LEN];
  uint8_t length;
  int8_t dirX, dirY;
  uint8_t foodX, foodY;
  uint8_t foodColor;
  uint16_t score;
  bool alive;
  unsigned long lastMoveMs;
  uint16_t moveIntervalMs;
};

#define SD_MAX_SPELLS 12
#define SD_PLAYER_W 12
#define SD_PLAYER_H 8

struct SDSpell {
  float x, y;
  uint8_t colorIdx;
  bool active;
};

struct SpellDodgeState {
  float playerX;
  SDSpell spells[SD_MAX_SPELLS];
  uint8_t lives;
  uint16_t score;
  float spellSpeed;
  unsigned long lastSpawnMs;
  unsigned long startMs;
  bool alive;
};

void mgInit(AppState game);
void mgUpdate(AppState game, const JoystickState& js);
void mgRender(AppState game);
bool mgIsAlive(AppState game);

#endif
