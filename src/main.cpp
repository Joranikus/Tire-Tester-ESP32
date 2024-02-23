#include <Arduino.h>
#include "Test.hpp"
#include "WheelSpeedEncoder.hpp"
#include "SwivelEncoder.hpp"
#include "MotorController.hpp"

//----------------------------------------------------------------------------------------
//----------------------------------------SETTINGS----------------------------------------
//----------------------------------------------------------------------------------------

//----------PARAMETERS----------
float wheel_diameter_mm = 74;
float wheel_width_mm = 30;
float motor_voltage = 12;
float distance_from_center_to_wheel_mm = 315;
String material_type = "TPU";

//--------TEST SETTINGS---------
float motor_speed = 100;
float acceleration_time = 0;

//----------------------------------------------------------------------------------------

MotorController motor(18, motor_voltage);
WheelSpeedEncoder wheel_encoder(32, 33, 1000, 0.25, wheel_diameter_mm);
SwivelEncoder swivel_encoder(27, 14, 500, 2, distance_from_center_to_wheel_mm);

Test test(motor,
          wheel_encoder,
          swivel_encoder,
          wheel_diameter_mm,
          wheel_width_mm,
          motor_voltage,
          distance_from_center_to_wheel_mm,
          material_type);

bool test_triggered = false;

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Initialized");
}

void run_test() {
    test.run_main_test(motor_speed, acceleration_time);
}

void loop() {
    if (Serial.available() > 0) {
        char command = Serial.read();
        if (command == 't') {
            test_triggered = true;
        }
    }

    if (test_triggered) {
        run_test();
        test_triggered = false;
    }
}
