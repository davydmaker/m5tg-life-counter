#include "display.h"
#include <M5Unified.h>

static M5Canvas sprite(&M5.Display);
static bool spriteReady = false;
static ColorTheme currentTheme;
static const ColorTheme* theme = &currentTheme;

void displaySetTheme(ThemeId id) {
  if (id < THEME_COUNT) {
    memcpy_P(&currentTheme, &THEMES[id], sizeof(ColorTheme));
  }
}

static void beginDraw(uint16_t bgColor = COLOR_BG) {
  if (!spriteReady) {
    sprite.createSprite(SCREEN_W, SCREEN_H);
    spriteReady = true;
  }
  sprite.fillSprite(bgColor);
}

static void endDraw() {
  sprite.pushSprite(0, 0);
}

static void drawCentered(const char* text, int y, uint8_t size, uint16_t color, uint16_t bg = COLOR_BG) {
  sprite.setTextSize(size);
  sprite.setTextColor(color, bg);
  int16_t tw = sprite.textWidth(text);
  sprite.setCursor((SCREEN_W - tw) / 2, y);
  sprite.print(text);
}

static void drawCenteredNum(int16_t num, int y, uint8_t size, uint16_t color) {
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", num);
  drawCentered(buf, y, size, color);
}

static void drawBattery(int x, int y, uint16_t bg = COLOR_BG) {
  int32_t batLevel = M5.Power.getBatteryLevel();

  if (batLevel < 0) batLevel = 0;
  if (batLevel > 100) batLevel = 100;

  sprite.drawRect(x, y, 16, 9, COLOR_DIM);
  sprite.fillRect(x + 16, y + 2, 2, 5, COLOR_DIM);

  uint16_t fillColor = COLOR_BATTERY;
  if (batLevel <= 10) fillColor = COLOR_BAT_CRIT;
  else if (batLevel <= 30) fillColor = COLOR_BAT_LOW;

  int fillW = (int)(12.0f * batLevel / 100.0f);
  if (fillW > 0) {
    sprite.fillRect(x + 2, y + 2, fillW, 5, fillColor);
  }

  char buf[8];
  snprintf(buf, sizeof(buf), "%d%%", batLevel);
  sprite.setTextSize(1);
  sprite.setTextColor(fillColor, bg);
  sprite.setCursor(x + 20, y + 1);
  sprite.print(buf);
}

void displayInit() {
  M5.Display.setRotation(1);
  M5.Display.setBrightness(DEFAULT_BRIGHTNESS);
  M5.Display.fillScreen(COLOR_BG);
  memcpy_P(&currentTheme, &THEMES[THEME_PLAINS], sizeof(ColorTheme));
}

void displayStartup() {
  beginDraw();
  drawCentered("MTG", 20, 4, MTG_WHITE);
  drawCentered("Life Counter", 65, 2, COLOR_DIM);

  uint16_t manaColors[] = {MTG_WHITE, MTG_BLUE, MTG_BLACK, MTG_RED, MTG_GREEN};
  int startX = (SCREEN_W - 5 * 16) / 2;
  for (int i = 0; i < 5; i++) {
    sprite.fillCircle(startX + i * 16 + 8, 105, 5, manaColors[i]);
  }
  endDraw();
}

void displayMainMenu(uint8_t selection) {
  beginDraw();
  drawCentered("MTG Life Counter", 15, 2, theme->title);
  drawBattery(SCREEN_W - 55, 2);

  const char* items[] = {"Start Game", "Options", "About"};
  uint8_t itemCount = MMENU_COUNT;
  uint8_t spacing = 28;
  int startY = 38;

  for (uint8_t i = 0; i < itemCount; i++) {
    int y = startY + i * spacing;
    bool sel = (i == selection);
    if (sel) {
      sprite.fillRoundRect(30, y - 4, SCREEN_W - 60, 20, 4, theme->selBg);
      drawCentered(items[i], y, 2, theme->selText, theme->selBg);
    } else {
      drawCentered(items[i], y, 2, COLOR_DIM);
    }
  }

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(30, 125);
  sprite.print("[OK] Select   [A] Navigate");
  endDraw();
}

void displayGameModeSelect(uint8_t selection) {
  beginDraw();
  drawCentered("Select Mode", 10, 2, theme->title);

  const char* options[] = {"Standard (20 LP)", "Commander (40 LP)", "Custom"};
  for (uint8_t i = 0; i < GMODE_COUNT; i++) {
    int y = 35 + i * 28;
    bool sel = (i == selection);
    if (sel) {
      sprite.fillRoundRect(20, y - 4, SCREEN_W - 40, 24, 4, theme->selBg);
      drawCentered(options[i], y, 2, theme->selText, theme->selBg);
    } else {
      drawCentered(options[i], y, 2, COLOR_DIM);
    }
  }

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(15, 122);
  sprite.print("[OK] Start  [A] Switch  [B] Back");
  endDraw();
}

void displayCustomLifeInput(uint8_t life) {
  beginDraw();
  drawCentered("Custom Starting Life", 10, 2, theme->title);

  sprite.setTextSize(4);
  sprite.setTextColor(theme->accent, COLOR_BG);
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", life);
  int16_t w = sprite.textWidth(buf);
  sprite.setCursor((SCREEN_W - w) / 2, 50);
  sprite.print(buf);

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(40, 90);
  sprite.print("(Range: 1-255)");

  sprite.setCursor(5, 108);
  sprite.print("[OK] +1  [A] -1");
  sprite.setCursor(5, 118);
  sprite.print("[Long] +5/-5  [B] Confirm");

  endDraw();
}

void displayPlayerThemeSelect(uint8_t playerIdx, ThemeId selectedTheme) {
  beginDraw();

  char title[16];
  snprintf(title, sizeof(title), "Player %d Theme", playerIdx + 1);
  drawCentered(title, 5, 2, theme->title);

  const char* themeNames[] = {"Plains", "Island", "Swamp", "Mountain", "Forest"};

  for (uint8_t i = 0; i < THEME_COUNT; i++) {
    int y = 28 + i * 18;
    bool sel = (i == selectedTheme);

    ColorTheme tempTheme;
    memcpy_P(&tempTheme, &THEMES[i], sizeof(ColorTheme));

    if (sel) {
      sprite.fillRoundRect(15, y - 3, SCREEN_W - 30, 16, 3, tempTheme.selBg);
      sprite.fillCircle(28, y + 4, 6, tempTheme.activeBar);
      sprite.drawCircle(28, y + 4, 6, tempTheme.selText);
      sprite.setTextSize(1);
      sprite.setTextColor(tempTheme.selText, tempTheme.selBg);
      sprite.setCursor(42, y);
      sprite.print(themeNames[i]);
    } else {
      sprite.fillCircle(28, y + 4, 6, tempTheme.activeBar);
      sprite.drawCircle(28, y + 4, 6, COLOR_DIM);
      sprite.setTextSize(1);
      sprite.setTextColor(COLOR_DIM, COLOR_BG);
      sprite.setCursor(42, y);
      sprite.print(themeNames[i]);
    }
  }

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(15, 120);
  sprite.print("[OK] Confirm  [A] Change");

  endDraw();
}

void displayGame(const GameState& gs, TimerMode timerMode) {
  beginDraw();

  int halfH = 58;
  int timerH = 18;

  for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
    int yBase = i * (halfH + 1);
    bool isActive = (i == gs.activePlayer);

    ColorTheme playerTheme;
    memcpy_P(&playerTheme, &THEMES[gs.players[i].theme], sizeof(ColorTheme));

    sprite.fillRect(0, yBase, SCREEN_W, halfH, playerTheme.menuBg);

    if (isActive) {
      uint16_t borderColor = playerTheme.activeBar;
      sprite.fillRect(0, yBase, SCREEN_W, 3, borderColor);
      sprite.fillRect(0, yBase, 3, halfH, borderColor);
      sprite.fillRect(SCREEN_W - 3, yBase, 3, halfH, borderColor);
      sprite.fillRect(0, yBase + halfH - 3, SCREEN_W, 3, borderColor);
    }

    char label[8];
    snprintf(label, sizeof(label), "P%d", i + 1);
    sprite.setTextSize(2);
    sprite.setTextColor(playerTheme.title, playerTheme.menuBg);
    sprite.setCursor(10, yBase + 8);
    sprite.print(label);

    uint16_t lifeColor = COLOR_TEXT;
    if (gs.players[i].life <= 5) lifeColor = COLOR_LIFE_CRIT;
    else if (gs.players[i].life <= 10) lifeColor = COLOR_LIFE_WARN;

    char lifeBuf[8];
    snprintf(lifeBuf, sizeof(lifeBuf), "%d", gs.players[i].life);
    sprite.setTextSize(4);
    sprite.setTextColor(lifeColor, playerTheme.menuBg);
    int16_t tw = sprite.textWidth(lifeBuf);
    sprite.setCursor((SCREEN_W - tw) / 2, yBase + 16);
    sprite.print(lifeBuf);
  }

  sprite.drawFastHLine(10, halfH, SCREEN_W - 20, COLOR_DIVIDER);

  int barY = SCREEN_H - timerH;
  sprite.drawFastHLine(0, barY, SCREEN_W, COLOR_DIVIDER);

  unsigned long secs = gameGetMatchSeconds(gs);
  char timerBuf[16];
  snprintf(timerBuf, sizeof(timerBuf), "%lu:%02lu", secs / 60, secs % 60);
  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(6, barY + 5);
  sprite.print(timerBuf);

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(SCREEN_W / 2 - 24, barY + 5);
  sprite.print("[B]=Menu");

  drawBattery(SCREEN_W - 55, barY + 4);

  endDraw();
}

void displayGameMenu(const GameState& gs, uint8_t selection) {
  beginDraw(theme->menuBg);
  drawCentered("= MENU =", 5, 2, theme->title, theme->menuBg);

  const char* items[] = {"Switch Player", "Roll Dice", "Flip Coin", "Settings", "Reset Game"};
  for (uint8_t i = 0; i < GMENU_COUNT; i++) {
    int y = 28 + i * 16;
    bool sel = (i == selection);
    uint16_t bg = sel ? theme->selBg : theme->menuBg;
    if (sel) {
      sprite.fillRoundRect(15, y - 2, SCREEN_W - 30, 15, 3, theme->selBg);
    }
    uint16_t color = sel ? theme->selText : COLOR_TEXT;
    drawCentered(items[i], y, 1, color, bg);
  }

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, theme->menuBg);
  sprite.setCursor(20, 125);
  sprite.print("[OK] OK  [A] Nav  [B] Back");
  endDraw();
}

void displayDice(const GameState& gs) {
  beginDraw();

  char title[16];
  snprintf(title, sizeof(title), "D%d", gs.diceType);
  drawCentered(title, 10, 2, theme->accent);

  if (gs.showingResult) {
    int boxSize = (gs.diceType == 100) ? 75 : 60;
    int bx = (SCREEN_W - boxSize) / 2;
    int by = 35;
    sprite.drawRoundRect(bx, by, boxSize, boxSize, 6, MTG_WHITE);
    sprite.drawRoundRect(bx + 1, by + 1, boxSize - 2, boxSize - 2, 5, MTG_WHITE);

    char buf[8];
    snprintf(buf, sizeof(buf), "%d", gs.lastDiceResult);
    uint8_t textSize = (gs.diceType == 100) ? 3 : 4;
    sprite.setTextSize(textSize);
    sprite.setTextColor(MTG_WHITE, COLOR_BG);
    int16_t tw = sprite.textWidth(buf);
    int th = (textSize == 4) ? 28 : 21;
    sprite.setCursor(bx + (boxSize - tw) / 2, by + (boxSize - th) / 2);
    sprite.print(buf);
  } else {
    drawCentered("Shake or [A]!", 55, 2, COLOR_DIM);
  }

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(5, 105);
  sprite.print("[OK] Change  [A] Roll  [B] Back");
  endDraw();
}

void displayCoin(const GameState& gs) {
  beginDraw();
  drawCentered("Coin Flip", 10, 2, theme->accent);

  if (gs.showingResult) {
    int cx = SCREEN_W / 2;
    int cy = 67;
    uint16_t color = gs.lastCoinResult ? MTG_WHITE : MTG_RED;
    sprite.fillCircle(cx, cy, 28, color);
    sprite.drawCircle(cx, cy, 28, MTG_WHITE);
    const char* result = gs.lastCoinResult ? "HEADS" : "TAILS";
    sprite.setTextSize(1);
    sprite.setTextColor(gs.lastCoinResult ? COLOR_BG : MTG_WHITE, color);
    int tw = sprite.textWidth(result);
    sprite.setCursor(cx - tw / 2, cy - 3);
    sprite.print(result);
  } else {
    drawCentered("Shake or [A]!", 55, 2, COLOR_DIM);
  }

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(15, 118);
  sprite.print("[A] Flip  [Shake]  [B] Back");
  endDraw();
}

void displayConfirmReset(uint8_t selection) {
  beginDraw();
  drawCentered("Reset Game?", 20, 2, MTG_RED);

  const char* opts[] = {"Cancel", "Reset"};
  for (int i = 0; i < 2; i++) {
    int y = 60 + i * 30;
    bool sel = (i == selection);
    if (sel) {
      sprite.fillRoundRect(50, y - 4, SCREEN_W - 100, 24, 4, theme->selBg);
      drawCentered(opts[i], y, 2, theme->selText, theme->selBg);
    } else {
      drawCentered(opts[i], y, 2, COLOR_DIM);
    }
  }

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(30, 122);
  sprite.print("[OK] Select   [A] Switch");
  endDraw();
}

void displayGameOver(const GameState& gs, TimerMode timerMode) {
  beginDraw();

  uint8_t winner = (gs.loserIndex == 0) ? 1 : 0;
  char buf[32];

  drawCentered("GAME OVER", 8, 2, MTG_RED);

  snprintf(buf, sizeof(buf), "Player %d Wins!", winner + 1);
  drawCentered(buf, 32, 2, MTG_WHITE);

  for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
    int y = 60 + i * 16;
    snprintf(buf, sizeof(buf), "P%d: %d LP", i + 1, gs.players[i].life);
    sprite.setTextSize(1);
    uint16_t color = (i == gs.loserIndex) ? MTG_RED : MTG_GREEN;
    sprite.setTextColor(color, COLOR_BG);
    sprite.setCursor(70, y);
    sprite.print(buf);
  }

  unsigned long matchSecs = (gs.matchStartMs > 0) ? ((millis() - gs.matchStartMs) / 1000) : 0;
  snprintf(buf, sizeof(buf), "Duration: %lu:%02lu", matchSecs / 60, matchSecs % 60);
  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(60, 100);
  sprite.print(buf);

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(30, 115);
  sprite.print("[OK] New Game  [B] Menu");
  endDraw();
}

void displaySettings(uint8_t selection, uint8_t brightness, uint8_t volume, TimerMode timerMode, ThemeId themeId, bool faceDownPause, uint8_t shutdownIdleIdx, uint8_t shutdownGameIdx) {
  beginDraw();
  drawCentered("Settings", 2, 2, theme->accent);

  const char* themeNames[] = {"Plains", "Island", "Swamp", "Mountain", "Forest"};

  for (uint8_t i = 0; i < SET_COUNT; i++) {
    int y = 18 + i * 13;
    bool sel = (i == selection);
    uint16_t bg = sel ? theme->selBg : COLOR_BG;
    uint16_t color = sel ? theme->selText : COLOR_DIM;

    if (sel) {
      sprite.fillRoundRect(8, y - 2, SCREEN_W - 16, 12, 2, theme->selBg);
    }

    sprite.setTextSize(1);
    sprite.setTextColor(color, bg);

    if (i == SET_BRIGHTNESS) {
      sprite.setCursor(14, y);
      sprite.print("Brightness");
      int barX = 90; int barW = 90;
      sprite.drawRect(barX, y, barW, 7, COLOR_DIM);
      int fillW = (int)((float)barW * brightness / 255.0f);
      if (fillW > 0) sprite.fillRect(barX + 1, y + 1, fillW - 1, 5, color);
      char pct[8];
      snprintf(pct, sizeof(pct), "%d%%", (int)(brightness * 100 / 255));
      sprite.setCursor(barX + barW + 4, y);
      sprite.print(pct);
    }
    else if (i == SET_VOLUME) {
      sprite.setCursor(14, y);
      sprite.print("Volume");
      int barX = 90; int barW = 90;
      sprite.drawRect(barX, y, barW, 7, COLOR_DIM);
      int fillW = (int)((float)barW * volume / 255.0f);
      if (fillW > 0) sprite.fillRect(barX + 1, y + 1, fillW - 1, 5, color);
      char pct[8];
      snprintf(pct, sizeof(pct), "%d%%", (int)(volume * 100 / 255));
      sprite.setCursor(barX + barW + 4, y);
      sprite.print(pct);
    }
    else if (i == SET_THEME) {
      sprite.setCursor(14, y);
      sprite.print("Theme:");
      sprite.setCursor(90, y);
      sprite.print(themeNames[themeId]);
    }
    else if (i == SET_FACE_DOWN_PAUSE) {
      sprite.setCursor(14, y);
      sprite.print("Face down:");
      sprite.setCursor(90, y);
      sprite.print(faceDownPause ? "ON" : "OFF");
    }
    else if (i == SET_SHUTDOWN_IDLE) {
      sprite.setCursor(14, y);
      sprite.print("Off idle:");
      char buf[8];
      snprintf(buf, sizeof(buf), "%d min", pgm_read_byte(&SHUTDOWN_IDLE_MIN[shutdownIdleIdx]));
      sprite.setCursor(90, y);
      sprite.print(buf);
    }
    else if (i == SET_SHUTDOWN_GAME) {
      sprite.setCursor(14, y);
      sprite.print("Off game:");
      char buf[8];
      snprintf(buf, sizeof(buf), "%d min", pgm_read_byte(&SHUTDOWN_GAME_MIN[shutdownGameIdx]));
      sprite.setCursor(90, y);
      sprite.print(buf);
    }
    else if (i == SET_DIAGNOSTICS) {
      sprite.setCursor(14, y);
      sprite.print("Diagnostics >");
    }
    else if (i == SET_BACK) {
      sprite.setCursor(14, y);
      sprite.print("< Back");
    }
  }

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(8, 125);
  sprite.print("[OK] Adjust  [A] Nav  [B] Back");
  endDraw();
}

void displayAbout() {
  beginDraw();
  drawCentered("About", 3, 2, theme->accent);

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);

  sprite.setCursor(20, 24);  sprite.print("MTG Life Counter v1.0");
  sprite.setCursor(20, 36);  sprite.print("For M5StickC PLUS 2");

  sprite.setTextColor(MTG_RED, COLOR_BG);
  sprite.setCursor(20, 54);  sprite.print("made with <3 by dvd");

  sprite.setTextColor(MTG_BLUE, COLOR_BG);
  sprite.setCursor(20, 66);  sprite.print("davydmaker.com");

  sprite.setTextColor(theme->accent, COLOR_BG);
  sprite.setCursor(20, 84);  sprite.print("Shoutout to UMotivo community!");

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(20, 100); sprite.print("A=+life B=-life Shake=swap");
  sprite.setCursor(20, 112); sprite.print("PWR=menu  Hold PWR=off");

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(60, 127);
  sprite.print("[B] Back");
  endDraw();
}

void displayDiceAnimation(uint8_t sides) {
  char title[8];
  snprintf(title, sizeof(title), "D%d", sides);
  for (int i = 0; i < 8; i++) {
    beginDraw();
    drawCentered(title, 10, 2, theme->accent);
    int boxSize = 60;
    int bx = (SCREEN_W - boxSize) / 2;
    int by = 35;
    sprite.drawRoundRect(bx, by, boxSize, boxSize, 6, COLOR_DIM);
    sprite.drawRoundRect(bx + 1, by + 1, boxSize - 2, boxSize - 2, 5, COLOR_DIM);

    int randomNum = random(1, sides + 1);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", randomNum);
    sprite.setTextSize(4);
    sprite.setTextColor(COLOR_DIM, COLOR_BG);
    int16_t tw = sprite.textWidth(buf);
    int th = 28;
    sprite.setCursor(bx + (boxSize - tw) / 2, by + (boxSize - th) / 2);
    sprite.print(buf);

    endDraw();
    delay(50 + i * 15);
  }
}

void displayCoinAnimation() {
  const char* faces[] = {"HEADS", "TAILS"};
  for (int i = 0; i < 6; i++) {
    beginDraw();
    drawCentered("Coin Flip", 10, 2, theme->accent);
    drawCentered(faces[i % 2], 55, 2, COLOR_DIM);
    endDraw();
    delay(80 + i * 20);
  }
}

void displayVictoryAnimation(uint8_t winnerIdx, const GameState& gs) {
  ColorTheme winnerTheme;
  memcpy_P(&winnerTheme, &THEMES[gs.players[winnerIdx].theme], sizeof(ColorTheme));

  for (int i = 0; i < 5; i++) {
    beginDraw();
    int radius = 20 + i * 15;
    sprite.drawCircle(SCREEN_W / 2, SCREEN_H / 2, radius, winnerTheme.activeBar);
    if (i > 0) {
      sprite.drawCircle(SCREEN_W / 2, SCREEN_H / 2, radius - 15, winnerTheme.accent);
    }
    endDraw();
    delay(100);
  }

  for (int i = 0; i < 10; i++) {
    beginDraw();
    char msg[16];
    snprintf(msg, sizeof(msg), "PLAYER %d", winnerIdx + 1);
    sprite.setTextSize(2);
    sprite.setTextColor(winnerTheme.activeBar, COLOR_BG);
    int16_t w = sprite.textWidth(msg);
    sprite.setCursor((SCREEN_W - w) / 2, 40);
    sprite.print(msg);

    sprite.setTextSize(3);
    sprite.setTextColor(winnerTheme.accent, COLOR_BG);
    w = sprite.textWidth("WINS!");
    sprite.setCursor((SCREEN_W - w) / 2, 70);
    sprite.print("WINS!");
    endDraw();
    delay(100);
  }

  for (int i = 0; i < 5; i++) {
    beginDraw(i % 2 == 0 ? COLOR_BG : winnerTheme.menuBg);
    endDraw();
    delay(100);
  }
}

void displayDiagnostics(uint8_t selection, bool hasEasterEggs) {
  beginDraw();
  drawCentered("Diagnostics", 5, 2, theme->accent);

  const char* allItems[] = {
    "Battery Info",
    "System Info",
    "Game Stats",
    "Temperature",
    "IMU Status",
    "Tests",
    "Easter Eggs",
    "< Back"
  };

  const char* visibleItems[DIAG_COUNT];
  uint8_t visibleCount = 0;
  uint8_t visibleSel = 0;

  for (uint8_t i = 0; i < DIAG_COUNT; i++) {
    if (i == DIAG_EASTER_EGGS && !hasEasterEggs) continue;
    if (i == selection) visibleSel = visibleCount;
    visibleItems[visibleCount++] = allItems[i];
  }

  int topMargin = 25;
  int availableHeight = SCREEN_H - topMargin - 20;
  int spacing = availableHeight / visibleCount;

  for (uint8_t i = 0; i < visibleCount; i++) {
    int y = topMargin + i * spacing;
    bool sel = (i == visibleSel);

    if (sel) {
      sprite.fillRoundRect(20, y - 3, SCREEN_W - 40, spacing - 2, 3, theme->selBg);
      sprite.setTextSize(1);
      sprite.setTextColor(theme->selText, theme->selBg);
      sprite.setCursor(30, y);
      sprite.print(visibleItems[i]);
    } else {
      sprite.setTextSize(1);
      sprite.setTextColor(COLOR_DIM, COLOR_BG);
      sprite.setCursor(30, y);
      sprite.print(visibleItems[i]);
    }
  }

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(20, 125);
  sprite.print("[OK] Select  [A] Nav  [B] Back");
  endDraw();
}

void displayBatteryInfo() {
  beginDraw();
  drawCentered("Battery Info", 3, 2, theme->accent);

  int32_t voltage = M5.Power.getBatteryVoltage();
  int32_t level = M5.Power.getBatteryLevel();

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);

  char buf[32];
  snprintf(buf, sizeof(buf), "Voltage: %ld mV", voltage);
  sprite.setCursor(20, 28);
  sprite.print(buf);

  float voltageV = voltage / 1000.0f;
  snprintf(buf, sizeof(buf), "         (%.2fV)", voltageV);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(20, 40);
  sprite.print(buf);


  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  snprintf(buf, sizeof(buf), "Level: %ld%%", level);
  sprite.setCursor(20, 54);
  sprite.print(buf);


  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(20, 68);
  sprite.print("Status:");

  uint16_t statusColor = MTG_GREEN;
  const char* statusText = "Normal";

  if (voltage < 3300) {
    statusColor = COLOR_BAT_CRIT;
    statusText = "Critical";
  } else if (voltage < 3600) {
    statusColor = COLOR_BAT_LOW;
    statusText = "Low";
  } else if (voltage > 4100) {
    statusColor = MTG_GREEN;
    statusText = "Full";
  }

  sprite.setTextColor(statusColor, COLOR_BG);
  sprite.setCursor(90, 68);
  sprite.print(statusText);

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(20, 86);  sprite.print("Reference (LiPo):");
  sprite.setCursor(20, 98);  sprite.print("4.2V = 100% (full)");
  sprite.setCursor(20, 110); sprite.print("3.7V = ~50% (nominal)");
  sprite.setCursor(20, 122); sprite.print("3.0V = 0% (empty)");

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(185, 3);
  sprite.print("[B]");
  endDraw();
}

void displaySystemInfo() {
  beginDraw();
  drawCentered("System Info", 3, 2, theme->accent);

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);

  sprite.setCursor(20, 22);
  sprite.print("Model:");
  sprite.setCursor(90, 22);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.print(APP_DEVICE);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 34);
  sprite.print("Version:");
  sprite.setCursor(90, 34);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.print(APP_VERSION);

  unsigned long uptimeSeconds = millis() / 1000;
  unsigned long days = uptimeSeconds / 86400;
  unsigned long hours = (uptimeSeconds % 86400) / 3600;
  unsigned long mins = (uptimeSeconds % 3600) / 60;

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 46);
  sprite.print("Uptime:");
  sprite.setCursor(90, 46);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  char uptime[32];
  if (days > 0) {
    snprintf(uptime, sizeof(uptime), "%lud %luh %lum", days, hours, mins);
  } else if (hours > 0) {
    snprintf(uptime, sizeof(uptime), "%luh %lum", hours, mins);
  } else {
    snprintf(uptime, sizeof(uptime), "%lum", mins);
  }
  sprite.print(uptime);

  uint32_t freeHeap = ESP.getFreeHeap();
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 58);
  sprite.print("Free RAM:");
  sprite.setCursor(90, 58);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  char ram[16];
  snprintf(ram, sizeof(ram), "%.1f KB", freeHeap / 1024.0f);
  sprite.print(ram);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 70);
  sprite.print("CPU Freq:");
  sprite.setCursor(90, 70);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.print(getCpuFrequencyMhz());
  sprite.print(" MHz");

  uint32_t flashSize = ESP.getFlashChipSize();
  uint32_t sketchSize = ESP.getSketchSize();
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 82);
  sprite.print("Flash:");
  sprite.setCursor(90, 82);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  char flash[24];
  snprintf(flash, sizeof(flash), "%lu/%lu KB", sketchSize/1024, flashSize/1024);
  sprite.print(flash);

  int barX = 20; int barY = 96; int barW = 200; int barH = 8;
  sprite.drawRect(barX, barY, barW, barH, COLOR_DIM);
  int fillW = (int)((float)barW * sketchSize / flashSize);
  if (fillW > 0) sprite.fillRect(barX + 1, barY + 1, fillW - 1, barH - 2, theme->accent);

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(185, 3);
  sprite.print("[B]");

  endDraw();
}

void displayGameStats(uint16_t totalMatches, uint32_t totalPlaytime,
                      uint16_t p1Wins, uint16_t p2Wins,
                      uint16_t diceRolls, uint16_t coinFlips) {
  beginDraw();
  drawCentered("Game Statistics", 3, 2, theme->accent);

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);

  sprite.setCursor(20, 24);
  sprite.print("Total matches:");
  sprite.setCursor(150, 24);
  sprite.setTextColor(theme->accent, COLOR_BG);
  sprite.print(totalMatches);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 36);
  sprite.print("Total time:");
  sprite.setCursor(150, 36);
  sprite.setTextColor(theme->accent, COLOR_BG);
  unsigned long hours = totalPlaytime / 3600;
  unsigned long mins = (totalPlaytime % 3600) / 60;
  char time[16];
  snprintf(time, sizeof(time), "%luh %lum", hours, mins);
  sprite.print(time);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 48);
  sprite.print("Avg duration:");
  sprite.setCursor(150, 48);
  sprite.setTextColor(theme->accent, COLOR_BG);
  if (totalMatches > 0) {
    unsigned long avgSeconds = totalPlaytime / totalMatches;
    unsigned long avgMins = avgSeconds / 60;
    char avg[16];
    snprintf(avg, sizeof(avg), "%lu min", avgMins);
    sprite.print(avg);
  } else {
    sprite.print("--");
  }

  sprite.drawLine(20, 62, SCREEN_W - 20, 62, COLOR_DIM);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 68);
  sprite.print("Player 1 wins:");
  sprite.setCursor(150, 68);
  sprite.setTextColor(MTG_GREEN, COLOR_BG);
  sprite.print(p1Wins);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 80);
  sprite.print("Player 2 wins:");
  sprite.setCursor(150, 80);
  sprite.setTextColor(MTG_BLUE, COLOR_BG);
  sprite.print(p2Wins);

  sprite.drawLine(20, 94, SCREEN_W - 20, 94, COLOR_DIM);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 100);
  sprite.print("Dice rolled:");
  sprite.setCursor(150, 100);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.print(diceRolls);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 112);
  sprite.print("Coins flipped:");
  sprite.setCursor(150, 112);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.print(coinFlips);

  sprite.setTextColor(COLOR_BAT_CRIT, COLOR_BG);
  sprite.setCursor(30, 125);
  sprite.print("Hold [OK] to RESET stats");

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(185, 3);
  sprite.print("[B]");

  endDraw();
}

void displayTemperature() {
  beginDraw();
  drawCentered("Temperature", 3, 2, theme->accent);

  float tempC = temperatureRead();

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);

  sprite.setCursor(20, 40);
  sprite.print("Internal temp:");

  sprite.setTextSize(3);
  sprite.setCursor(60, 60);

  uint16_t tempColor = MTG_GREEN;
  const char* statusText = "Normal";

  if (tempC > 70) {
    tempColor = COLOR_BAT_CRIT;
    statusText = "Hot!";
  } else if (tempC > 60) {
    tempColor = COLOR_BAT_LOW;
    statusText = "Warm";
  }

  sprite.setTextColor(tempColor, COLOR_BG);
  char temp[8];
  snprintf(temp, sizeof(temp), "%.0f", tempC);
  sprite.print(temp);
  sprite.setTextSize(2);
  sprite.print("C");

  sprite.setTextSize(1);
  sprite.setCursor(85, 100);
  sprite.setTextColor(tempColor, COLOR_BG);
  sprite.print(statusText);

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(20, 115);
  sprite.print("Safe range: < 60");
  sprite.print((char)247);
  sprite.print("C");

  sprite.setCursor(185, 3);
  sprite.print("[B]");

  endDraw();
}

void displayIMUStatus() {
  beginDraw();
  drawCentered("IMU Status", 3, 2, theme->accent);

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);

  sprite.setCursor(20, 22);
  sprite.print("Mode:");
  sprite.setCursor(90, 22);
  sprite.setTextColor(MTG_GREEN, COLOR_BG);
  sprite.print("Active");

  float accX, accY, accZ;
  M5.Imu.getAccel(&accX, &accY, &accZ);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 40);
  sprite.print("Accel X:");
  sprite.setCursor(90, 40);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.2f G", accX);
  sprite.print(buf);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 52);
  sprite.print("Accel Y:");
  sprite.setCursor(90, 52);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  snprintf(buf, sizeof(buf), "%.2f G", accY);
  sprite.print(buf);

  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 64);
  sprite.print("Accel Z:");
  sprite.setCursor(90, 64);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  snprintf(buf, sizeof(buf), "%.2f G", accZ);
  sprite.print(buf);

  float magnitude = sqrt(accX*accX + accY*accY + accZ*accZ);
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(20, 76);
  sprite.print("Magnitude:");
  sprite.setCursor(90, 76);
  sprite.setTextColor(theme->accent, COLOR_BG);
  snprintf(buf, sizeof(buf), "%.2f G", magnitude);
  sprite.print(buf);

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(20, 94);
  sprite.print("Shake threshold: ");
  sprite.print(SHAKE_THRESHOLD);
  sprite.print(" G");

  sprite.setCursor(20, 106);
  if (magnitude > SHAKE_THRESHOLD) {
    sprite.setTextColor(MTG_RED, COLOR_BG);
    sprite.print(">> SHAKE DETECTED!");
  } else {
    sprite.setTextColor(MTG_GREEN, COLOR_BG);
    sprite.print("   Stable");
  }

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(185, 3);
  sprite.print("[B]");

  endDraw();
}

void displayTestMenu(uint8_t selection) {
  beginDraw();
  drawCentered("Tests", 5, 2, theme->accent);

  const char* items[] = {
    "IMU Calibration",
    "Button Test",
    "Screen Test",
    "Speaker Test",
    "< Back"
  };

  int topMargin = 30;
  int spacing = 18;

  for (uint8_t i = 0; i < TEST_COUNT; i++) {
    int y = topMargin + i * spacing;
    bool sel = (i == selection);

    if (sel) {
      sprite.fillRoundRect(20, y - 3, SCREEN_W - 40, spacing - 2, 3, theme->selBg);
      sprite.setTextSize(1);
      sprite.setTextColor(theme->selText, theme->selBg);
      sprite.setCursor(30, y);
      sprite.print(items[i]);
    } else {
      sprite.setTextSize(1);
      sprite.setTextColor(COLOR_DIM, COLOR_BG);
      sprite.setCursor(30, y);
      sprite.print(items[i]);
    }
  }

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(20, 125);
  sprite.print("[OK] Select  [A] Nav  [B] Back");
  endDraw();
}

void displayIMUCalibration(bool inProgress, uint8_t samplesCollected, float magnitude) {
  beginDraw();
  drawCentered("IMU Calibration", 5, 2, theme->accent);

  sprite.setTextSize(1);
  if (inProgress) {
    sprite.setTextColor(theme->accent, COLOR_BG);
    sprite.setCursor(30, 40);
    sprite.print("Calibrating...");

    sprite.setTextColor(COLOR_TEXT, COLOR_BG);
    sprite.setCursor(30, 60);
    sprite.printf("Samples: %d / 20", samplesCollected);

    int barWidth = (SCREEN_W - 60) * samplesCollected / 20;
    sprite.drawRect(30, 75, SCREEN_W - 60, 10, theme->accent);
    sprite.fillRect(30, 75, barWidth, 10, theme->accent);
  } else {
    sprite.setTextColor(COLOR_TEXT, COLOR_BG);
    sprite.setCursor(30, 40);
    sprite.print("Place device flat");
    sprite.setCursor(30, 55);
    sprite.print("on stable surface");

    sprite.setTextColor(theme->accent, COLOR_BG);
    sprite.setCursor(30, 80);
    sprite.printf("Current: %.2fG", magnitude);

    sprite.setTextColor(COLOR_DIM, COLOR_BG);
    sprite.setCursor(30, 100);
    sprite.print("Press [OK] to start");
  }

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(185, 3);
  sprite.print("[B]");
  endDraw();
}

void displayButtonTest(bool btnA, bool btnB, bool btnPWR) {
  beginDraw();
  drawCentered("Button Test", 5, 2, theme->accent);

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(30, 35);
  sprite.print("Press buttons to test");

  sprite.setCursor(30, 55);
  sprite.print("Button A:");
  if (btnA) {
    sprite.fillCircle(110, 59, 6, MTG_GREEN);
  } else {
    sprite.drawCircle(110, 59, 6, COLOR_DIM);
  }

  sprite.setCursor(30, 75);
  sprite.print("Button B:");
  if (btnB) {
    sprite.fillCircle(110, 79, 6, MTG_GREEN);
  } else {
    sprite.drawCircle(110, 79, 6, COLOR_DIM);
  }

  sprite.setCursor(30, 95);
  sprite.print("Power:");
  if (btnPWR) {
    sprite.fillCircle(110, 99, 6, MTG_GREEN);
  } else {
    sprite.drawCircle(110, 99, 6, COLOR_DIM);
  }

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(30, 120);
  sprite.print("[B] Hold to exit");
  endDraw();
}

void displayScreenTest(uint8_t pattern) {
  beginDraw();

  switch (pattern) {
    case 0:
      sprite.fillScreen(MTG_RED);
      sprite.setTextColor(COLOR_TEXT, MTG_RED);
      sprite.setCursor(10, 5);
      sprite.print("RED");
      break;
    case 1:
      sprite.fillScreen(MTG_GREEN);
      sprite.setTextColor(COLOR_BG, MTG_GREEN);
      sprite.setCursor(10, 5);
      sprite.print("GREEN");
      break;
    case 2:
      sprite.fillScreen(MTG_BLUE);
      sprite.setTextColor(COLOR_TEXT, MTG_BLUE);
      sprite.setCursor(10, 5);
      sprite.print("BLUE");
      break;
    case 3:
      sprite.fillScreen(MTG_WHITE);
      sprite.setTextColor(COLOR_BG, MTG_WHITE);
      sprite.setCursor(10, 5);
      sprite.print("WHITE");
      break;
    case 4:
      sprite.fillScreen(COLOR_BG);
      sprite.setTextColor(COLOR_TEXT, COLOR_BG);
      sprite.setCursor(10, 5);
      sprite.print("BLACK");
      break;
    case 5:
      sprite.fillScreen(COLOR_BG);
      for (int x = 0; x < SCREEN_W; x += 20) {
        sprite.drawLine(x, 0, x, SCREEN_H, COLOR_DIM);
      }
      for (int y = 0; y < SCREEN_H; y += 20) {
        sprite.drawLine(0, y, SCREEN_W, y, COLOR_DIM);
      }
      sprite.setTextColor(COLOR_TEXT, COLOR_BG);
      sprite.setCursor(10, 5);
      sprite.print("GRID");
      break;
  }

  sprite.setTextColor(COLOR_DIM, sprite.readPixel(185, 3));
  sprite.setCursor(145, 125);
  sprite.print("[A] Next");
  sprite.setCursor(185, 3);
  sprite.print("[B]");
  endDraw();
}

void displaySpeakerTest(uint16_t frequency) {
  beginDraw();
  drawCentered("Speaker Test", 5, 2, theme->accent);

  sprite.setTextSize(2);
  sprite.setTextColor(theme->accent, COLOR_BG);
  sprite.setCursor(60, 50);
  sprite.printf("%d Hz", frequency);

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_TEXT, COLOR_BG);
  sprite.setCursor(30, 80);
  sprite.print("Use [OK]/[A] to adjust");
  sprite.setCursor(30, 95);
  sprite.print("frequency");

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(30, 120);
  sprite.print("[OK] +100  [A] -100  [B] Exit");

  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(185, 3);
  sprite.print("[B]");
  endDraw();
}

M5Canvas& displayGetSprite() { return sprite; }
void displayBeginDraw(uint16_t bg) { beginDraw(bg); }
void displayEndDraw() { endDraw(); }
void displayDrawCentered(const char* t, int y, uint8_t s, uint16_t c, uint16_t bg) {
  drawCentered(t, y, s, c, bg);
}
const ColorTheme* displayGetTheme() { return theme; }

void displayEasterEggsMenu(uint8_t selection) {
  beginDraw();
  drawCentered("Easter Eggs", 5, 2, 0xFEA0);

  const char* items[] = {"Mana Runner", "Arena Battle", "Snake", "Spell Dodge", "< Back"};
  int spacing = 18;
  int topY = 30;

  for (uint8_t i = 0; i < EE_COUNT; i++) {
    int y = topY + i * spacing;
    bool sel = (i == selection);

    if (sel) {
      sprite.fillRoundRect(20, y - 3, SCREEN_W - 40, 16, 3, theme->selBg);
      sprite.setTextSize(1);
      sprite.setTextColor(theme->selText, theme->selBg);
      sprite.setCursor(35, y);
      sprite.print(items[i]);
    } else {
      sprite.setTextSize(1);
      sprite.setTextColor(COLOR_TEXT, COLOR_BG);
      sprite.setCursor(35, y);
      sprite.print(items[i]);
    }
  }

  uint16_t manaColors[] = {MTG_WHITE, MTG_BLUE, MTG_BLACK, MTG_RED, MTG_GREEN};
  int startX = (SCREEN_W - 5 * 20) / 2;
  for (int i = 0; i < 5; i++) {
    sprite.fillCircle(startX + i * 20 + 10, 128, 4, manaColors[i]);
  }

  endDraw();
}

void displayMiniGameOver(uint16_t score) {
  beginDraw();
  drawCentered("GAME OVER", 25, 3, MTG_RED);

  char buf[16];
  snprintf(buf, sizeof(buf), "Score: %d", score);
  drawCentered(buf, 65, 2, MTG_WHITE);

  sprite.setTextSize(1);
  sprite.setTextColor(COLOR_DIM, COLOR_BG);
  sprite.setCursor(40, 100);
  sprite.print("Press any button to exit");
  endDraw();
}

