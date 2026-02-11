#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>

#define JOYSTICK_I2C_ADDR  0x38
#define JOYSTICK_REG_CONVERTED  0x02
#define JOYSTICK_DEADZONE  40

struct JoystickState {
  int8_t x;
  int8_t y;
  bool button;
  bool connected;
};

void joystickInit();
bool joystickDetect();
void joystickRead(JoystickState& js);
int8_t joystickDirX(const JoystickState& js);
int8_t joystickDirY(const JoystickState& js);

#endif
