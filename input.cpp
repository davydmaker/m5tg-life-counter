#include "input.h"
#include <M5Unified.h>
#include <math.h>

#define BASELINE_SAMPLES 20

void inputInit(InputState& is) {
  memset(&is, 0, sizeof(InputState));
  is.baselineSet = false;
  is.baselineMag = 1.0f;

  float sum = 0;
  int validSamples = 0;
  for (int i = 0; i < BASELINE_SAMPLES; i++) {
    M5.update();
    M5.Imu.update();
    m5::imu_data_t d;
    M5.Imu.getImuData(&d);
    float m = sqrtf(d.accel.x * d.accel.x +
                    d.accel.y * d.accel.y +
                    d.accel.z * d.accel.z);
    if (m > 0.1f) {
      sum += m;
      validSamples++;
    }
    delay(15);
  }
  if (validSamples > 0) {
    is.baselineMag = sum / validSamples;
    is.baselineSet = true;
  }

}

InputEvent inputUpdate(InputState& is) {
  unsigned long now = millis();

  if (M5.BtnPWR.wasClicked()) {
    return INPUT_PWR;
  }

  M5.Imu.update();
  m5::imu_data_t imuData;
  M5.Imu.getImuData(&imuData);
  float mag = sqrtf(imuData.accel.x * imuData.accel.x +
                    imuData.accel.y * imuData.accel.y +
                    imuData.accel.z * imuData.accel.z);

  float deviation = fabsf(mag - is.baselineMag);
  if (deviation > SHAKE_THRESHOLD && (now - is.lastShakeMs) > SHAKE_COOLDOWN_MS) {
    is.lastShakeMs = now;
    return INPUT_SHAKE;
  }

  if (M5.BtnA.wasPressed()) {
    is.aHeld = true;
    is.aHeldSince = now;
    is.aLongFired = false;
    is.lastRepeatA = now;
    return INPUT_A_PRESS;
  }
  if (M5.BtnA.wasReleased()) {
    is.aHeld = false;
    is.aLongFired = false;
  }
  if (is.aHeld && !is.aLongFired && (now - is.aHeldSince) > LONG_PRESS_MS) {
    is.aLongFired = true;
    is.lastRepeatA = now;
    return INPUT_A_LONG;
  }
  if (is.aHeld && is.aLongFired && (now - is.lastRepeatA) > REPEAT_DELAY_MS) {
    is.lastRepeatA = now;
    return INPUT_A_LONG;
  }

  if (M5.BtnB.wasPressed()) {
    is.bHeld = true;
    is.bHeldSince = now;
    is.bLongFired = false;
    is.lastRepeatB = now;
    return INPUT_B_PRESS;
  }
  if (M5.BtnB.wasReleased()) {
    is.bHeld = false;
    is.bLongFired = false;
  }
  if (is.bHeld && !is.bLongFired && (now - is.bHeldSince) > LONG_PRESS_MS) {
    is.bLongFired = true;
    is.lastRepeatB = now;
    return INPUT_B_LONG;
  }
  if (is.bHeld && is.bLongFired && (now - is.lastRepeatB) > REPEAT_DELAY_MS) {
    is.lastRepeatB = now;
    return INPUT_B_LONG;
  }

  return INPUT_NONE;
}
