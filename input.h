#ifndef INPUT_H
#define INPUT_H

#include "config.h"
#include <Arduino.h>

enum InputEvent {
  INPUT_NONE,
  INPUT_A_PRESS,
  INPUT_B_PRESS,
  INPUT_A_LONG,
  INPUT_B_LONG,
  INPUT_PWR,
  INPUT_SHAKE
};

struct InputState {
  bool aHeld;
  bool bHeld;
  unsigned long aHeldSince;
  unsigned long bHeldSince;
  bool aLongFired;
  bool bLongFired;
  unsigned long lastRepeatA;
  unsigned long lastRepeatB;
  unsigned long lastShakeMs;
  float baselineMag;
  bool baselineSet;
};

void inputInit(InputState& is);
InputEvent inputUpdate(InputState& is);

#endif
