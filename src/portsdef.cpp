#include "Custom/portDef.hpp"

MotorGroup intake({19, -18}, pros::v5::MotorGears::rpm_200,
                  pros::v5::MotorUnits::degrees);

MotorGroup aleft({-11, 12}, pros::v5::MotorGears::blue,
                 pros::v5::MotorUnits::degrees);
MotorGroup aright({1, -2}, pros::v5::MotorGears::blue,
                  pros::v5::MotorUnits::degrees);
Controller userInput(E_CONTROLLER_MASTER);