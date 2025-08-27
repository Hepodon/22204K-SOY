#include "Custom/driveVoids.hpp"
#include "Custom/portDef.hpp"
#include "main.h"
#include "pros/abstract_motor.hpp"
#include "pros/misc.h"

using namespace pros;

float applySlew(int current, int target, float rate) {
  int diff = target - current;
  if (abs(diff) > rate)
    return current + rate * (diff > 0 ? 1 : -1);
  return target;
}

void intakeControl() {
  int inputIntake = 0;
  while (true) {
    if (userInput.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X)) {
      if (inputIntake == 1) {
        toggleControl(Stop);
        inputIntake = 0;
      } else {
        toggleControl(In);
        inputIntake = 1;
      }
    } else if (userInput.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y)) {
      if (inputIntake == 2) {
        toggleControl(Stop);
        inputIntake = 0;
      } else {
        toggleControl(Out);
        inputIntake = 2;
      }
    }
    delay(20);
  }
}

void middleControl() {
  int middleChain = 0;
  while (true) {
    if (userInput.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A)) {
      if (middleChain == 1) {
        toggleControl(Stop);
        middleChain = 0;
      } else {
        toggleControl(In);
        middleChain = 1;
      }
    } else if (userInput.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B)) {
      if (middleChain == 2) {
        toggleControl(Stop);
        middleChain = 0;
      } else {
        toggleControl(Out);
        middleChain = 2;
      }
    }
    delay(20);
  }
}

// Niam = Not In A Match

enum startingPos { Red_Left, Red_Right, Blue_Left, Blue_Right, NIAM };

void toggleControl(Direction dih, bool middle = false) {
  if (middle) {
    if (dih == In) {
      intake.move(127);
    } else if (dih == Out) {
      intake.move(-127);
    } else if (dih == Stop) {
      intake.brake();
    }
  } else {
    if (dih == In) {
      midChain.move(127);
    } else if (dih == Out) {
      midChain.move(-127);
    } else if (dih == Stop) {
      midChain.brake();
    }
  }
}
