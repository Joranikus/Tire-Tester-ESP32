#ifndef DEKKTESTER_TEST_HPP
#define DEKKTESTER_TEST_HPP

#include "MotorController.hpp"
#include "WheelSpeedEncoder.hpp"
#include "SwivelEncoder.hpp"

#define MAX_DATA_POINTS 1000 // Maximum number of data points to collect

class Test {
private:
    MotorController& motor_;
    WheelSpeedEncoder& wheel_encoder_;
    SwivelEncoder& swivel_encoder_;
    const unsigned int pre_motor_start_delay_ms_ = 1000; // Time before motor starts after test begins
    const unsigned int test_duration_ms_ = 5000; // Total duration of the test
    unsigned long acceleration_end_time_ = 0; // Time when acceleration ends
    bool is_accelerating_ = false; // Is motor currently accelerating

    // Array to store collected data points
    struct DataPoint {
        float wheel_position;
        float swivel_position;
        unsigned long timestamp;
    } data_points_[MAX_DATA_POINTS];
    int num_data_points_ = 0; // Current number of data points collected

public:
    Test(MotorController& motor, WheelSpeedEncoder& wheel_encoder, SwivelEncoder& swivel_encoder)
            : motor_(motor), wheel_encoder_(wheel_encoder), swivel_encoder_(swivel_encoder) {}

    void calibrate() {
        motor_.speed(50, 0); // Example calibration step
        delay(500);
        motor_.speed(0, 5); // Stop motor after brief run
        delay(500); // Stabilization delay
        swivel_encoder_.reset_distance();
        wheel_encoder_.reset_distance();
    }

    void run_main_test(float speed_percent, unsigned int acceleration_time_s) {
        Serial.println("RUN_CALIBRATION");
        calibrate();

        delay(1000); // Delay between calibration and test start for visibility

        unsigned long test_start_time = millis();
        unsigned long motor_start_time = test_start_time + pre_motor_start_delay_ms_;
        unsigned long next_data_collection_time = test_start_time;
        unsigned long current_time = 0;
        is_accelerating_ = true;
        acceleration_end_time_ = motor_start_time + (acceleration_time_s * 1000);

        Serial.println("READY_FOR_DATA_COLLECTION");
        Serial.println("START_TEST");

        while (millis() - test_start_time < test_duration_ms_) {
            current_time = millis();

            if (is_accelerating_ && current_time >= motor_start_time && current_time <= acceleration_end_time_) {
                float incremental_speed = speed_percent * float(current_time - motor_start_time) / float(acceleration_time_s * 1000);
                motor_.speed(incremental_speed, 0); // Set motor speed proportional to elapsed time
                if (current_time >= acceleration_end_time_) {
                    is_accelerating_ = false;
                    motor_.speed(speed_percent, 0); // Ensure motor is at target speed after acceleration
                }
            }

            if (current_time >= next_data_collection_time) {
                collect_data(current_time - test_start_time);
                next_data_collection_time += 100; // Collect data every 100ms
            }
        }

        motor_.speed(0, 0); // Stop motor at the end of the test
        Serial.println("END_TEST");

        // Send collected data to Python
        send_data_to_python();
    }

    void collect_data(unsigned long elapsed_time) {
        // Collect data points into the array
        data_points_[num_data_points_].wheel_position = wheel_encoder_.get_total_distance_m();
        data_points_[num_data_points_].swivel_position = swivel_encoder_.get_total_distance_m();
        data_points_[num_data_points_].timestamp = elapsed_time;

        // Increment the number of data points collected
        num_data_points_++;

        // Ensure not to exceed the maximum number of data points
        if (num_data_points_ >= MAX_DATA_POINTS) {
            Serial.println("Max data points reached. Stopping data collection.");
            while (true) {
                // Do nothing, or implement some error handling
            }
        }
    }

    void send_data_to_python() {
        // Send collected data to Python over Serial
        for (int i = 0; i < num_data_points_; ++i) {
            Serial.print("Data: ");
            Serial.print(data_points_[i].timestamp); Serial.print(",");
            Serial.print(data_points_[i].wheel_position); Serial.print(",");
            Serial.println(data_points_[i].swivel_position);
        }
    }
};

#endif // DEKKTESTER_TEST_HPP
