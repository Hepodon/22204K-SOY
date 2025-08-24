#include "main.h"
#include "Custom/Thermal.hpp"
#include "Custom/driveVoids.hpp"
#include "Custom/portDef.hpp"
#include "ThermalDef.cpp"
#include "pros/rtos.h"

void initialize() {
  c::task_create(intakeControl, NULL, 1, TASK_STACK_DEPTH_DEFAULT,
                 "Intake Control Task");
  build_ui();
}

void disabled() {}

void competition_initialize() {}

void autonomous() {}

void opcontrol() {

  float driveValue = 0;
  float turnValue = 0;

  const float driveSlewRate = 8;
  const float turnSlewRate = 16;

  int leftPower = 0;
  int rightPower = 0;

  while (true) {
    float rawDrive = userInput.get_analog(E_CONTROLLER_ANALOG_LEFT_Y);
    float rawTurn = userInput.get_analog(E_CONTROLLER_ANALOG_RIGHT_X);

    driveValue = applySlew(driveValue, rawDrive, driveSlewRate);
    turnValue = applySlew(turnValue, rawTurn, turnSlewRate);

    leftPower = driveValue + turnValue;
    rightPower = driveValue - turnValue;

    aleft.move(leftPower);
    aright.move(rightPower);

    if (userInput.get_digital(E_CONTROLLER_DIGITAL_R1)) {
      aleft.brake();
      aright.brake();
      driveValue = 0;
      turnValue = 0;
    }

    delay(20);
  }
}