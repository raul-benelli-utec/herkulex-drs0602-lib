#ifndef POSES_H
#define POSES_H

#include <Arduino.h>

// Estructura para definir una pose del brazo
struct Pose {
  const char* name;
  uint16_t pos[5];
};

// Declaraciones externas de las poses predefinidas
// Las definiciones están en src/poses.cpp
extern const Pose POSE_INICIAL;
extern const Pose POSE_TRABAJO;
extern const Pose POSE_TRABAJO_2;
extern const Pose POSE_STANDBY;

#endif // POSES_H

