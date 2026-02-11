#include "joystick.h"
#include <Wire.h>

void joystickInit() {
  Wire.begin(0, 26, 100000);
}

bool joystickDetect() {
  Wire.beginTransmission(JOYSTICK_I2C_ADDR);
  return (Wire.endTransmission() == 0);
}

void joystickRead(JoystickState& js) {
  Wire.beginTransmission(JOYSTICK_I2C_ADDR);
  Wire.write(JOYSTICK_REG_CONVERTED);
  Wire.endTransmission(false);

  Wire.requestFrom((int)JOYSTICK_I2C_ADDR, 3);
  if (Wire.available() >= 3) {
    js.x = (int8_t)Wire.read();
    js.y = (int8_t)Wire.read();
    js.button = (Wire.read() == 0);
    js.connected = true;
  } else {
    js.connected = false;
  }
}

int8_t joystickDirX(const JoystickState& js) {
  if (js.y > JOYSTICK_DEADZONE) return -1;
  if (js.y < -JOYSTICK_DEADZONE) return 1;
  return 0;
}

int8_t joystickDirY(const JoystickState& js) {
  if (js.x > JOYSTICK_DEADZONE) return -1;
  if (js.x < -JOYSTICK_DEADZONE) return 1;
  return 0;
}
