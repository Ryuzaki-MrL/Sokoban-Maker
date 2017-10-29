#ifndef ROBOT_H
#define ROBOT_H

#include "entity.h"

void initRobot(int x, int y);
void resetRobot(int x, int y);
void updateRobot();

int getRobotX();
int getRobotY();

#endif // ROBOT_H