#pragma once

#ifndef CUSTOM_DRIVEVOIDS_H
#define CUSTOM_DRIVEVOIDS_H

enum Direction { Forward, Reverse, Stop, Fwd, Rev, In, Out };

void toggleControl(Direction dih);
void intakeControl();
void middleControl();

float applySlew(int current, int target, float rate = 5);

#endif