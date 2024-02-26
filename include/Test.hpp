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
    float wheel_diameter_mm_;
    float wheel_width_mm_;
    float motor_voltage_;
    float distance_from_center_to_wheel_mm_;
    String material_type_;
    const unsigned int pre_motor_start_delay_ms_ = 1000; // Time before motor starts after test begins
    const unsigned int test_duration_ms_ = 5000; // Total duration of the test

    struct DataPoint {
        float wheel_position;
        float swivel_position;
        unsigned long timestamp;
    } data_points_[MAX_DATA_POINTS];
    int num_data_points_ = 0; // Current number of data points collected

public:
    Test(MotorController& motor, WheelSpeedEncoder& wheel_encoder, SwivelEncoder& swivel_encoder,
         float wheel_diameter_mm, float wheel_width_mm, float motor_voltage,
         float distance_from_center_to_wheel_mm, String material_type)
            : motor_(motor), wheel_encoder_(wheel_encoder), swivel_encoder_(swivel_encoder),
              wheel_diameter_mm_(wheel_diameter_mm), wheel_width_mm_(wheel_width_mm),
              motor_voltage_(motor_voltage), distance_from_center_to_wheel_mm_(distance_from_center_to_wheel_mm),
              material_type_(material_type) {}

    void calibrate() {
        motor_.speed(50, 0); // Example calibration step
        delay(50);
        motor_.speed(0, 3); // Stop motor after brief run
        delay(300); // Stabilization delay

        // Reset distance for both wheel and swivel encoders
        swivel_encoder_.reset_distance();
        wheel_encoder_.reset_distance();
    }

    void run_main_test(float speed_percent, unsigned int acceleration_time_s) {
        // Serial communication for calibration and test start
        Serial.println("RUN_CALIBRATION");
        calibrate();
        Serial.println("READY_FOR_DATA_COLLECTION");
        Serial.println("START_TEST");

        unsigned long test_start_time = millis();
        unsigned long motor_start_time = test_start_time + pre_motor_start_delay_ms_;
        unsigned long next_data_collection_time = test_start_time;
        unsigned long current_time = 0;

        while (true) {
            current_time = millis();

            // Start motor only after 1 second into the test
            if (current_time >= motor_start_time && !motor_.is_motor_running()) {
                motor_.speed(speed_percent, acceleration_time_s);
            }

            if (current_time >= next_data_collection_time) {
                collect_data(current_time - test_start_time);
                next_data_collection_time += 50; // Collect data every 100ms
            }

            // Check if the test duration has been reached
            if (current_time - test_start_time >= test_duration_ms_) {
                break;  // Exit the loop when the test duration is reached
            }
        }

        // Stop motor at the end of the test
        motor_.speed(0, 1);
        Serial.println("END_TEST");

        // Serial communication to send collected data to CSV
        send_data_to_serial();
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

    void send_data_to_serial() {

        Serial.println("---------------start---------------");

        Serial.println("Parameters:");
        Serial.print("Wheel Diameter (mm): ");
        Serial.println(wheel_diameter_mm_);
        Serial.print("Wheel Width (mm): ");
        Serial.println(wheel_width_mm_);
        Serial.print("Motor Voltage (V): ");
        Serial.println(motor_voltage_);
        Serial.print("Distance From Center to Wheel (mm): ");
        Serial.println(distance_from_center_to_wheel_mm_);
        Serial.print("Material Type: ");
        Serial.println(material_type_);
        Serial.println();

        // Serial communication to print data headers
        Serial.println("Timestamp (ms),Wheel Position (m),Swivel Position (m)");

        // Serial communication to print collected data
        for (int i = 0; i < num_data_points_; ++i) {
            Serial.print(data_points_[i].timestamp);
            Serial.print(",");
            Serial.print(data_points_[i].wheel_position);
            Serial.print(",");
            Serial.println(data_points_[i].swivel_position);
        }

        // Serial communication to print delimiter line after data
        Serial.println("---------------end---------------");
    }
};

#endif // DEKKTESTER_TEST_HPP