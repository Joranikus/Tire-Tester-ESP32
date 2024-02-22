#ifndef DEKKTESTER_SWIVELENCODER_HPP
#define DEKKTESTER_SWIVELENCODER_HPP

#include <Arduino.h>
#include <ESP32Encoder.h>

class SwivelEncoder {
private:
    int64_t previous_position = 0;
    unsigned long previous_time = 0;
    float gear_ratio;
    int ppr;
    float distance_from_center_to_wheel_mm;
    float last_speed_m_s = 0;
    float total_distance_m = 0;
    unsigned long last_speed_time = 0; // To store the time when the last speed was measured
    float last_acceleration_m_s2 = 0; // To store the last measured acceleration

public:
    SwivelEncoder(int pin_a, int pin_b, int ppr, float gear_ratio, float distance_from_center_to_wheel_mm)
            : ppr(ppr),
              gear_ratio(gear_ratio),
              distance_from_center_to_wheel_mm(distance_from_center_to_wheel_mm)
    {
        // Attach pins for use with encoder
        encoder.attachSingleEdge(pin_a, pin_b);
        previous_position = encoder.getCount();
        previous_time = millis();
        last_speed_time = previous_time; // Initialize last speed time
    }

    void set_distance_from_center_to_wheel(float distance) {
        distance_from_center_to_wheel_mm = distance;
    }

    float get_speed_m_s() {
        unsigned long current_time = millis();
        int64_t current_position = encoder.getCount();
        float time_elapsed = (current_time - previous_time) / 1000.0;
        int64_t position_change = current_position - previous_position;

        float angle_radians = (position_change / (float)ppr) * 2 * PI;
        float distance_moved = (distance_from_center_to_wheel_mm / 1000.0) * angle_radians;

        total_distance_m += abs(distance_moved);
        previous_position = current_position;
        previous_time = current_time;

        float speed_m_s = distance_moved / time_elapsed;
        last_speed_m_s = speed_m_s;
        last_speed_time = current_time;

        return abs(speed_m_s);
    }

    float get_total_distance_m() {
        int64_t current_position = encoder.getCount();
        float angle_radians = (current_position - previous_position) * 2 * PI / static_cast<float>(ppr);
        previous_position = current_position; // Update for the next call

        float distance_moved = (distance_from_center_to_wheel_mm / 1000.0) * angle_radians / gear_ratio;
        total_distance_m += abs(distance_moved);

        return abs(total_distance_m);
    }

    float calculateDistanceMoved(int64_t position_change) {
        float angle_radians = (position_change / (float)ppr) * 2 * PI;
        return (distance_from_center_to_wheel_mm / 1000.0) * angle_radians;
    }

    void reset_distance() {
        total_distance_m = 0;
    }

    float get_acceleration_m_s2() {
        float current_speed_m_s = get_speed_m_s(); // Get current speed
        unsigned long current_time = millis(); // Get current time
        float time_elapsed = (current_time - last_speed_time) / 1000.0; // Time elapsed since last speed measurement

        // Check to prevent division by zero
        if (time_elapsed == 0) return last_acceleration_m_s2;

        float acceleration = (current_speed_m_s - last_speed_m_s) / time_elapsed; // Calculate acceleration
        last_acceleration_m_s2 = acceleration; // Update last acceleration
        return acceleration; // Return the calculated acceleration
    }

    ESP32Encoder encoder;
};

#endif // DEKKTESTER_SWIVELENCODER_HPP
