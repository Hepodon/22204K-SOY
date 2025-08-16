#pragma once

#ifndef CUSTOM_DRIVEVOIDS_H
#define CUSTOM_DRIVEVOIDS_H

enum Direction { Forward, Reverse, Stop, Fwd, Rev, In, Out };

void toggleIntake(Direction dih);
void intakeControl(void*);

float applySlew(int current, int target, float rate = 5);

#endif