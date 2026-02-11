#include "minigames.h"
#include "display.h"
#include "audio.h"

static const uint16_t MANA_COLORS[] = {MTG_WHITE, MTG_BLUE, MTG_RED, MTG_GREEN, 0xC8DF};
static const uint8_t MANA_COLOR_COUNT = 5;

#define AB_ATTACK_RADIUS  20
#define GRID_DOT_COLOR    0x1082

static ManaRunnerState mrState;
static ArenaBattleState abState;
static SnakeState snState;
static SpellDodgeState sdState;

static void manaRunnerInit() {
  memset(&mrState, 0, sizeof(mrState));
  mrState.playerY = MR_PLAY_Y + MR_PLAY_H / 2;
  mrState.speed = 2.0f;
  mrState.alive = true;
  mrState.startMs = millis();
  mrState.lastSpawnMs = millis();
}

static void manaRunnerSpawnObstacle() {
  for (int i = 0; i < MR_MAX_OBSTACLES; i++) {
    if (!mrState.obstacles[i].active) {
      mrState.obstacles[i].active = true;
      mrState.obstacles[i].x = SCREEN_W + 10;
      mrState.obstacles[i].gapY = MR_PLAY_Y + 25 + random(MR_PLAY_H - 50);
      mrState.obstacles[i].gapSize = 40 - min((int)(mrState.speed), 8);
      if (mrState.obstacles[i].gapSize < 28) mrState.obstacles[i].gapSize = 28;
      return;
    }
  }
}

static void manaRunnerSpawnMana() {
  for (int i = 0; i < MR_MAX_MANA; i++) {
    if (!mrState.mana[i].active) {
      mrState.mana[i].active = true;
      mrState.mana[i].x = SCREEN_W + 10;
      mrState.mana[i].y = MR_PLAY_Y + 10 + random(MR_PLAY_H - 20);
      mrState.mana[i].colorIdx = random(MANA_COLOR_COUNT);
      return;
    }
  }
}

static void manaRunnerUpdate(const JoystickState& js) {
  if (!mrState.alive) return;

  int8_t dy = joystickDirY(js);
  mrState.playerY += dy * 3.0f;
  if (mrState.playerY < MR_PLAY_Y + 4) mrState.playerY = MR_PLAY_Y + 4;
  if (mrState.playerY > MR_PLAY_Y + MR_PLAY_H - MR_PLAYER_SIZE - 4)
    mrState.playerY = MR_PLAY_Y + MR_PLAY_H - MR_PLAYER_SIZE - 4;


  unsigned long elapsed = millis() - mrState.startMs;
  mrState.speed = 2.0f + elapsed / 5000.0f;
  if (mrState.speed > 6.0f) mrState.speed = 6.0f;


  if (millis() - mrState.lastSpawnMs > (unsigned long)(1200 / mrState.speed * 2)) {
    manaRunnerSpawnObstacle();
    if (random(3) == 0) manaRunnerSpawnMana();
    mrState.lastSpawnMs = millis();
  }

  float px = 30;
  float py = mrState.playerY;


  for (int i = 0; i < MR_MAX_OBSTACLES; i++) {
    if (!mrState.obstacles[i].active) continue;
    mrState.obstacles[i].x -= mrState.speed;
    if (mrState.obstacles[i].x < -20) {
      mrState.obstacles[i].active = false;
      mrState.score++;
      continue;
    }


    float ox = mrState.obstacles[i].x;
    float gapTop = mrState.obstacles[i].gapY - mrState.obstacles[i].gapSize / 2;
    float gapBot = mrState.obstacles[i].gapY + mrState.obstacles[i].gapSize / 2;

    if (px + MR_PLAYER_SIZE > ox && px < ox + 12) {
      if (py < gapTop || py + MR_PLAYER_SIZE > gapBot) {
        mrState.alive = false;
        audioGameOver();
        return;
      }
    }
  }


  for (int i = 0; i < MR_MAX_MANA; i++) {
    if (!mrState.mana[i].active) continue;
    mrState.mana[i].x -= mrState.speed * 0.8f;
    if (mrState.mana[i].x < -10) {
      mrState.mana[i].active = false;
      continue;
    }


    float mx = mrState.mana[i].x;
    float my = mrState.mana[i].y;
    if (abs(px + 4 - mx) < 10 && abs(py + 4 - my) < 10) {
      mrState.mana[i].active = false;
      mrState.score += 5;
      audioGamePoint();
    }
  }
}

static void manaRunnerRender() {
  M5Canvas& spr = displayGetSprite();
  displayBeginDraw();


  spr.setTextSize(1);
  spr.setTextColor(COLOR_TEXT, COLOR_BG);
  spr.setCursor(5, 3);
  spr.print("Mana Runner");
  char buf[16];
  snprintf(buf, sizeof(buf), "Score: %d", mrState.score);
  spr.setCursor(160, 3);
  spr.print(buf);


  spr.drawFastHLine(0, MR_PLAY_Y - 1, SCREEN_W, COLOR_DIVIDER);

  if (!mrState.alive) {
    displayMiniGameOver(mrState.score);
    return;
  }


  for (int i = 0; i < MR_MAX_OBSTACLES; i++) {
    if (!mrState.obstacles[i].active) continue;
    int ox = (int)mrState.obstacles[i].x;
    int gapTop = mrState.obstacles[i].gapY - mrState.obstacles[i].gapSize / 2;
    int gapBot = mrState.obstacles[i].gapY + mrState.obstacles[i].gapSize / 2;


    if (gapTop > MR_PLAY_Y)
      spr.fillRect(ox, MR_PLAY_Y, 12, gapTop - MR_PLAY_Y, MTG_RED);

    if (gapBot < MR_PLAY_Y + MR_PLAY_H)
      spr.fillRect(ox, gapBot, 12, MR_PLAY_Y + MR_PLAY_H - gapBot, MTG_RED);
  }


  for (int i = 0; i < MR_MAX_MANA; i++) {
    if (!mrState.mana[i].active) continue;
    spr.fillCircle((int)mrState.mana[i].x, (int)mrState.mana[i].y, 4,
                   MANA_COLORS[mrState.mana[i].colorIdx]);
  }

  spr.fillRect(30, (int)mrState.playerY, MR_PLAYER_SIZE, MR_PLAYER_SIZE, MTG_WHITE);
  spr.drawRect(30, (int)mrState.playerY, MR_PLAYER_SIZE, MR_PLAYER_SIZE, MTG_BLUE);

  displayEndDraw();
}

static void arenaSpawnEnemy() {
  for (int i = 0; i < AB_MAX_ENEMIES; i++) {
    if (!abState.enemies[i].alive) {
      abState.enemies[i].alive = true;
      abState.enemies[i].colorIdx = random(MANA_COLOR_COUNT);


      uint8_t edge = random(4);
      switch (edge) {
        case 0:
          abState.enemies[i].x = random(SCREEN_W);
          abState.enemies[i].y = AB_PLAY_Y;
          break;
        case 1:
          abState.enemies[i].x = random(SCREEN_W);
          abState.enemies[i].y = AB_PLAY_Y + AB_PLAY_H - 6;
          break;
        case 2:
          abState.enemies[i].x = 0;
          abState.enemies[i].y = AB_PLAY_Y + random(AB_PLAY_H);
          break;
        case 3:
          abState.enemies[i].x = SCREEN_W - 6;
          abState.enemies[i].y = AB_PLAY_Y + random(AB_PLAY_H);
          break;
      }
      return;
    }
  }
}

static void arenaInit() {
  memset(&abState, 0, sizeof(abState));
  abState.playerX = SCREEN_W / 2;
  abState.playerY = AB_PLAY_Y + AB_PLAY_H / 2;
  abState.hp = 3;
  abState.wave = 1;
  abState.alive = true;
  abState.lastSpawnMs = millis();
  abState.lastHitMs = 0;


  for (int i = 0; i < 3; i++) arenaSpawnEnemy();
}

static void arenaUpdate(const JoystickState& js) {
  if (!abState.alive) return;


  int8_t dx = joystickDirX(js);
  int8_t dy = joystickDirY(js);
  abState.playerX += dx * 3.0f;
  abState.playerY += dy * 3.0f;


  if (abState.playerX < 4) abState.playerX = 4;
  if (abState.playerX > SCREEN_W - 10) abState.playerX = SCREEN_W - 10;
  if (abState.playerY < AB_PLAY_Y + 4) abState.playerY = AB_PLAY_Y + 4;
  if (abState.playerY > AB_PLAY_Y + AB_PLAY_H - 10) abState.playerY = AB_PLAY_Y + AB_PLAY_H - 10;


  if (js.button && !abState.attacking) {
    abState.attacking = true;
    abState.attackStartMs = millis();
    audioGameAttack();


    for (int i = 0; i < AB_MAX_ENEMIES; i++) {
      if (!abState.enemies[i].alive) continue;
      float edx = abState.enemies[i].x - abState.playerX;
      float edy = abState.enemies[i].y - abState.playerY;
      if (edx * edx + edy * edy < AB_ATTACK_RADIUS * AB_ATTACK_RADIUS) {
        abState.enemies[i].alive = false;
        abState.score += 10;
        audioGamePoint();
      }
    }
  }


  if (abState.attacking && millis() - abState.attackStartMs > 200) {
    abState.attacking = false;
  }


  float enemySpeed = 0.8f + abState.wave * 0.2f;
  if (enemySpeed > 3.0f) enemySpeed = 3.0f;

  bool invincible = (millis() - abState.lastHitMs < 1000);

  for (int i = 0; i < AB_MAX_ENEMIES; i++) {
    if (!abState.enemies[i].alive) continue;

    float edx = abState.playerX - abState.enemies[i].x;
    float edy = abState.playerY - abState.enemies[i].y;
    float dist = sqrt(edx * edx + edy * edy);
    if (dist > 1) {
      abState.enemies[i].x += (edx / dist) * enemySpeed;
      abState.enemies[i].y += (edy / dist) * enemySpeed;
    }


    if (!invincible && dist < 8) {
      abState.hp--;
      abState.lastHitMs = millis();
      audioGameHit();
      abState.enemies[i].alive = false;
      if (abState.hp == 0) {
        abState.alive = false;
        audioGameOver();
        return;
      }
    }
  }


  unsigned long spawnBase = (abState.wave * 200 < 2000) ? (2000 - abState.wave * 200) : 0;
  unsigned long spawnInterval = (spawnBase > 800) ? spawnBase : 800;
  if (millis() - abState.lastSpawnMs > spawnInterval) {
    abState.lastSpawnMs = millis();
    arenaSpawnEnemy();

    uint8_t newWave = 1 + abState.score / 50;
    if (newWave > abState.wave) {
      abState.wave = newWave;
    }
  }
}

static void arenaRender() {
  M5Canvas& spr = displayGetSprite();
  displayBeginDraw();


  spr.setTextSize(1);
  spr.setTextColor(COLOR_TEXT, COLOR_BG);
  spr.setCursor(5, 3);
  char buf[24];
  snprintf(buf, sizeof(buf), "HP:%d  Wave:%d", abState.hp, abState.wave);
  spr.print(buf);
  snprintf(buf, sizeof(buf), "Score: %d", abState.score);
  spr.setCursor(160, 3);
  spr.print(buf);

  spr.drawFastHLine(0, AB_PLAY_Y - 1, SCREEN_W, COLOR_DIVIDER);

  if (!abState.alive) {
    displayMiniGameOver(abState.score);
    return;
  }


  for (int i = 0; i < AB_MAX_ENEMIES; i++) {
    if (!abState.enemies[i].alive) continue;
    spr.fillRect((int)abState.enemies[i].x, (int)abState.enemies[i].y, 6, 6,
                 MANA_COLORS[abState.enemies[i].colorIdx]);
  }


  if (abState.attacking) {
    uint16_t attackColor = 0xFEA0;
    spr.drawCircle((int)abState.playerX + 3, (int)abState.playerY + 3, 18, attackColor);
    spr.drawCircle((int)abState.playerX + 3, (int)abState.playerY + 3, 20, attackColor);
  }

  bool invincible = (millis() - abState.lastHitMs < 1000);
  if (!invincible || (millis() / 100) % 2 == 0) {
    spr.fillCircle((int)abState.playerX + 3, (int)abState.playerY + 3, 5, MTG_WHITE);
    spr.drawCircle((int)abState.playerX + 3, (int)abState.playerY + 3, 5, MTG_BLUE);
  }

  displayEndDraw();
}

static void snakePlaceFood() {
  bool valid;
  do {
    valid = true;
    snState.foodX = random(SNAKE_COLS);
    snState.foodY = random(SNAKE_ROWS);


    for (int i = 0; i < snState.length; i++) {
      if (snState.bodyX[i] == snState.foodX && snState.bodyY[i] == snState.foodY) {
        valid = false;
        break;
      }
    }
  } while (!valid);
  snState.foodColor = random(MANA_COLOR_COUNT);
}

static void snakeInit() {
  memset(&snState, 0, sizeof(snState));
  snState.length = 3;
  snState.dirX = 1;
  snState.dirY = 0;
  snState.alive = true;
  snState.moveIntervalMs = 200;
  snState.lastMoveMs = millis();


  for (int i = 0; i < 3; i++) {
    snState.bodyX[i] = SNAKE_COLS / 2 - i;
    snState.bodyY[i] = SNAKE_ROWS / 2;
  }

  snakePlaceFood();
}

static void snakeUpdate(const JoystickState& js) {
  if (!snState.alive) return;

  int8_t dx = joystickDirX(js);
  int8_t dy = joystickDirY(js);

  if (dx != 0 && snState.dirX == 0) {
    snState.dirX = dx;
    snState.dirY = 0;
  } else if (dy != 0 && snState.dirY == 0) {
    snState.dirY = dy;
    snState.dirX = 0;
  }


  if (millis() - snState.lastMoveMs < snState.moveIntervalMs) return;
  snState.lastMoveMs = millis();


  int8_t newX = snState.bodyX[0] + snState.dirX;
  int8_t newY = snState.bodyY[0] + snState.dirY;

  if (newX < 0) newX = SNAKE_COLS - 1;
  if (newX >= SNAKE_COLS) newX = 0;
  if (newY < 0) newY = SNAKE_ROWS - 1;
  if (newY >= SNAKE_ROWS) newY = 0;


  for (int i = 0; i < snState.length; i++) {
    if (snState.bodyX[i] == newX && snState.bodyY[i] == newY) {
      snState.alive = false;
      audioGameOver();
      return;
    }
  }


  bool ate = (newX == snState.foodX && newY == snState.foodY);

  int newLen = ate ? snState.length + 1 : snState.length;
  if (newLen > SNAKE_MAX_LEN) newLen = SNAKE_MAX_LEN;

  for (int i = newLen - 1; i > 0; i--) {
    snState.bodyX[i] = snState.bodyX[i - 1];
    snState.bodyY[i] = snState.bodyY[i - 1];
  }
  snState.bodyX[0] = newX;
  snState.bodyY[0] = newY;
  snState.length = newLen;

  if (ate) {
    snState.score += 10;
    audioGamePoint();
    snakePlaceFood();


    if (snState.moveIntervalMs > 80) {
      snState.moveIntervalMs -= 5;
    }
  }
}

static void snakeRender() {
  M5Canvas& spr = displayGetSprite();
  displayBeginDraw();


  spr.setTextSize(1);
  spr.setTextColor(COLOR_TEXT, COLOR_BG);
  spr.setCursor(5, 3);
  spr.print("Snake");
  char buf[16];
  snprintf(buf, sizeof(buf), "Score: %d", snState.score);
  spr.setCursor(160, 3);
  spr.print(buf);

  spr.drawFastHLine(0, SNAKE_OFFSET_Y - 1, SCREEN_W, COLOR_DIVIDER);

  if (!snState.alive) {
    displayMiniGameOver(snState.score);
    return;
  }


  for (int x = 0; x < SNAKE_COLS; x++) {
    for (int y = 0; y < SNAKE_ROWS; y++) {
      int px = x * SNAKE_CELL;
      int py = SNAKE_OFFSET_Y + y * SNAKE_CELL;
      spr.drawPixel(px, py, GRID_DOT_COLOR);
    }
  }


  int fx = snState.foodX * SNAKE_CELL + SNAKE_CELL / 2;
  int fy = SNAKE_OFFSET_Y + snState.foodY * SNAKE_CELL + SNAKE_CELL / 2;
  spr.fillCircle(fx, fy, 4, MANA_COLORS[snState.foodColor]);
  spr.drawCircle(fx, fy, 4, COLOR_TEXT);


  for (int i = 0; i < snState.length; i++) {
    int sx = snState.bodyX[i] * SNAKE_CELL + 1;
    int sy = SNAKE_OFFSET_Y + snState.bodyY[i] * SNAKE_CELL + 1;
    uint16_t color = (i == 0) ? MTG_WHITE : MTG_GREEN;
    spr.fillRect(sx, sy, SNAKE_CELL - 2, SNAKE_CELL - 2, color);
  }

  displayEndDraw();
}

static void spellDodgeInit() {
  memset(&sdState, 0, sizeof(sdState));
  sdState.playerX = SCREEN_W / 2 - SD_PLAYER_W / 2;
  sdState.lives = 3;
  sdState.spellSpeed = 1.5f;
  sdState.alive = true;
  sdState.startMs = millis();
  sdState.lastSpawnMs = millis();
}

static void spellDodgeUpdate(const JoystickState& js) {
  if (!sdState.alive) return;


  int8_t dx = joystickDirX(js);
  sdState.playerX += dx * 4.0f;
  if (sdState.playerX < 0) sdState.playerX = 0;
  if (sdState.playerX > SCREEN_W - SD_PLAYER_W) sdState.playerX = SCREEN_W - SD_PLAYER_W;


  unsigned long elapsed = millis() - sdState.startMs;
  sdState.spellSpeed = 1.5f + elapsed / 8000.0f;
  if (sdState.spellSpeed > 5.0f) sdState.spellSpeed = 5.0f;


  unsigned long spawnBase = (elapsed / 20 < 500) ? (500 - elapsed / 20) : 0;
  unsigned long spawnInterval = (spawnBase > 200) ? spawnBase : 200;
  if (millis() - sdState.lastSpawnMs > spawnInterval) {
    sdState.lastSpawnMs = millis();
    for (int i = 0; i < SD_MAX_SPELLS; i++) {
      if (!sdState.spells[i].active) {
        sdState.spells[i].active = true;
        sdState.spells[i].x = random(SCREEN_W - 8);
        sdState.spells[i].y = 14;
        sdState.spells[i].colorIdx = random(MANA_COLOR_COUNT);
        break;
      }
    }
  }

  float py = SCREEN_H - 20;


  for (int i = 0; i < SD_MAX_SPELLS; i++) {
    if (!sdState.spells[i].active) continue;
    sdState.spells[i].y += sdState.spellSpeed;


    if (sdState.spells[i].y > SCREEN_H) {
      sdState.spells[i].active = false;
      sdState.score++;
      continue;
    }


    float sx = sdState.spells[i].x;
    float sy = sdState.spells[i].y;
    if (sx + 8 > sdState.playerX && sx < sdState.playerX + SD_PLAYER_W &&
        sy + 8 > py && sy < py + SD_PLAYER_H) {
      sdState.spells[i].active = false;
      sdState.lives--;
      audioGameHit();
      if (sdState.lives == 0) {
        sdState.alive = false;
        audioGameOver();
        return;
      }
    }
  }
}

static void spellDodgeRender() {
  M5Canvas& spr = displayGetSprite();
  displayBeginDraw();


  spr.setTextSize(1);
  spr.setTextColor(COLOR_TEXT, COLOR_BG);
  spr.setCursor(5, 3);
  spr.print("Spell Dodge");


  for (int i = 0; i < sdState.lives; i++) {
    spr.fillCircle(150 + i * 14, 6, 4, MTG_RED);
  }

  char buf[16];
  snprintf(buf, sizeof(buf), "%d", sdState.score);
  spr.setCursor(210, 3);
  spr.print(buf);

  spr.drawFastHLine(0, 13, SCREEN_W, COLOR_DIVIDER);

  if (!sdState.alive) {
    displayMiniGameOver(sdState.score);
    return;
  }


  for (int i = 0; i < SD_MAX_SPELLS; i++) {
    if (!sdState.spells[i].active) continue;
    spr.fillRect((int)sdState.spells[i].x, (int)sdState.spells[i].y, 8, 8,
                 MANA_COLORS[sdState.spells[i].colorIdx]);
  }

  float py = SCREEN_H - 20;
  spr.fillRect((int)sdState.playerX, (int)py, SD_PLAYER_W, SD_PLAYER_H, MTG_WHITE);
  spr.drawRect((int)sdState.playerX, (int)py, SD_PLAYER_W, SD_PLAYER_H, MTG_BLUE);

  displayEndDraw();
}

void mgInit(AppState game) {
  switch (game) {
    case STATE_GAME_MANA_RUNNER: manaRunnerInit(); break;
    case STATE_GAME_ARENA:       arenaInit();      break;
    case STATE_GAME_SNAKE:       snakeInit();      break;
    case STATE_GAME_SPELL_DODGE: spellDodgeInit(); break;
    default: break;
  }
}

void mgUpdate(AppState game, const JoystickState& js) {
  switch (game) {
    case STATE_GAME_MANA_RUNNER: manaRunnerUpdate(js); break;
    case STATE_GAME_ARENA:       arenaUpdate(js);      break;
    case STATE_GAME_SNAKE:       snakeUpdate(js);      break;
    case STATE_GAME_SPELL_DODGE: spellDodgeUpdate(js); break;
    default: break;
  }
}

void mgRender(AppState game) {
  switch (game) {
    case STATE_GAME_MANA_RUNNER: manaRunnerRender(); break;
    case STATE_GAME_ARENA:       arenaRender();      break;
    case STATE_GAME_SNAKE:       snakeRender();      break;
    case STATE_GAME_SPELL_DODGE: spellDodgeRender(); break;
    default: break;
  }
}

bool mgIsAlive(AppState game) {
  switch (game) {
    case STATE_GAME_MANA_RUNNER: return mrState.alive;
    case STATE_GAME_ARENA:       return abState.alive;
    case STATE_GAME_SNAKE:       return snState.alive;
    case STATE_GAME_SPELL_DODGE: return sdState.alive;
    default: return false;
  }
}

