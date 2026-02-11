#include "audio.h"
#include <M5Unified.h>

void audioInit() {
  M5.Speaker.setVolume(SPEAKER_VOLUME);
}

void audioLifeUp() {
  M5.Speaker.tone(TONE_LIFE_UP, TONE_DURATION);
}

void audioLifeDown() {
  M5.Speaker.tone(TONE_LIFE_DOWN, TONE_DURATION);
}

void audioDiceRoll() {
  for (int i = 0; i < 5; i++) {
    M5.Speaker.tone(800 + i * 100, 40);
    delay(50);
  }
  M5.Speaker.tone(TONE_DICE_ROLL, 150);
}

void audioCoinFlip() {
  M5.Speaker.tone(TONE_COIN_FLIP, 60);
  delay(80);
  M5.Speaker.tone(TONE_COIN_FLIP + 200, 60);
  delay(80);
  M5.Speaker.tone(TONE_COIN_FLIP + 400, 100);
}

void audioConfirm() {
  M5.Speaker.tone(TONE_CONFIRM, 100);
}

void audioDefeat() {
  M5.Speaker.tone(440, 200);
  delay(220);
  M5.Speaker.tone(370, 200);
  delay(220);
  M5.Speaker.tone(330, 200);
  delay(220);
  M5.Speaker.tone(TONE_DEFEAT, 400);
}

void audioStartup() {
  M5.Speaker.tone(523, 80);
  delay(100);
  M5.Speaker.tone(659, 80);
  delay(100);
  M5.Speaker.tone(784, 80);
  delay(100);
  M5.Speaker.tone(1047, 150);
}

void audioVictory() {
  M5.Speaker.tone(523, 120);
  delay(140);
  M5.Speaker.tone(659, 120);
  delay(140);
  M5.Speaker.tone(784, 120);
  delay(140);
  M5.Speaker.tone(1047, 300);
  delay(320);
  M5.Speaker.tone(1047, 200);
}

void audioGamePoint() {
  M5.Speaker.tone(1318, 50);
}

void audioGameHit() {
  M5.Speaker.tone(200, 100);
}

void audioGameOver() {
  M5.Speaker.tone(392, 150);
  delay(170);
  M5.Speaker.tone(330, 150);
  delay(170);
  M5.Speaker.tone(262, 300);
}

void audioGameAttack() {
  M5.Speaker.tone(1500, 40);
  delay(50);
  M5.Speaker.tone(1800, 40);
}

