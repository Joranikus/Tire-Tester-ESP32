#ifndef DEKKTESTER_TEST_HPP
#define DEKKTESTER_TEST_HPP

#include "MotorController.hpp"
#include "WheelSpeedEncoder.hpp"
#include "SwivelEncoder.hpp"

class Test {
private:
    MotorController& motor;
    WheelSpeedEncoder& wheel_encoder;
    SwivelEncoder& swivel_encoder;
    const unsigned int pre_motor_start_delay_ms = 1000; // Time before motor starts after test begins
    const unsigned int test_duration_ms = 5000; // Total duration of the test
    unsigned long accelerationEndTime = 0; // Time when acceleration ends
    bool isAccelerating = false; // Is motor currently accelerating

public:
    Test(MotorController& m, WheelSpeedEncoder& we, SwivelEncoder& se)
            : motor(m), wheel_encoder(we), swivel_encoder(se) {}

    void calibrate() {
        motor.speed(50, 0); // Example calibration step
        delay(500);
        motor.speed(0, 5); // Stop motor after brief run
        delay(500); // Stabilization delay
        swivel_encoder.reset_distance();
        wheel_encoder.reset_distance();
    }

    void run_main_test(float speed_percent, unsigned int acceleration_time_s) {
        Serial.println("RUN_CALIBRATION");
        calibrate();

        delay(1000); // Delay between calibration and test start for visibility

        unsigned long testStartTime = millis();
        unsigned long motorStartTime = testStartTime + pre_motor_start_delay_ms;
        unsigned long nextDataCollectionTime = testStartTime;
        unsigned long currentTime = 0;
        isAccelerating = true;
        accelerationEndTime = motorStartTime + (acceleration_time_s * 1000);

        Serial.println("READY_FOR_DATA_COLLECTION");
        Serial.println("START_TEST");

        while (millis() - testStartTime < test_duration_ms) {
            currentTime = millis();

            if (isAccelerating && currentTime >= motorStartTime && currentTime <= accelerationEndTime) {
                float incrementalSpeed = speed_percent * float(currentTime - motorStartTime) / float(acceleration_time_s * 1000);
                motor.speed(incrementalSpeed, 0); // Set motor speed proportional to elapsed time
                if (currentTime >= accelerationEndTime) {
                    isAccelerating = false;
                    motor.speed(speed_percent, 0); // Ensure motor is at target speed after acceleration
                }
            }

            if (currentTime >= nextDataCollectionTime) {
                collectAndPrintData(currentTime - testStartTime);
                nextDataCollectionTime += 100; // Collect data every 100ms
            }
        }

        motor.speed(0, 0); // Stop motor at the end of the test
        Serial.println("END_TEST");
    }

    void collectAndPrintData(unsigned long elapsed_time) {
        float wheel_speed = wheel_encoder.get_speed_m_s();
        float wheel_distance = wheel_encoder.get_total_distance_m();
        float wheel_acceleration = wheel_encoder.get_acceleration_m_s2();

        float swivel_speed = swivel_encoder.get_speed_m_s();
        float swivel_distance = swivel_encoder.get_total_distance_m();
        float swivel_acceleration = swivel_encoder.get_acceleration_m_s2();

        Serial.print("Data: ");
        Serial.print(elapsed_time); Serial.print(",");
        Serial.print(wheel_speed); Serial.print(",");
        Serial.print(wheel_distance); Serial.print(",");
        Serial.print(wheel_acceleration); Serial.print(",");
        Serial.print(swivel_speed); Serial.print(",");
        Serial.print(swivel_distance); Serial.print(",");
        Serial.println(swivel_acceleration);
    }
};

#endif // DEKKTESTER_TEST_HPP