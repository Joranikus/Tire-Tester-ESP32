#ifndef DEKKTESTER_WHEELSPEEDENCODER_HPP
#define DEKKTESTER_WHEELSPEEDENCODER_HPP

#include <Arduino.h>
#include <ESP32Encoder.h>

class WheelSpeedEncoder {
private:
    int64_t previous_position = 0;
    unsigned long previous_time = 0;
    float gear_ratio;
    int ppr;
    float wheel_diameter_mm;
    float last_speed_m_s = 0;
    float total_distance_m = 0;

public:
    WheelSpeedEncoder(int pin_a, int pin_b, int ppr, float gear_ratio, float wheel_diameter_mm)
            : ppr(ppr),
              gear_ratio(gear_ratio),
              wheel_diameter_mm(wheel_diameter_mm)
    {
        // Attach pins for use with encoder
        encoder.attachSingleEdge(pin_a, pin_b);
        previous_position = encoder.getCount();
        previous_time = millis();
    }

    void set_wheel_diameter(float wheel_diameter) {
        wheel_diameter_mm = wheel_diameter;
    }

    float get_speed_m_s() {
        unsigned long current_time = millis();
        int64_t current_position = encoder.getCount();
        float time_elapsed = (current_time - previous_time) / 1000.0;

        int64_t position_change = current_position - previous_position;
        previous_position = current_position;
        previous_time = current_time;

        float wheel_circumference = PI * (wheel_diameter_mm / 1000.0);
        float rotations = (position_change / (float)ppr) / gear_ratio;
        float distance_moved = rotations * wheel_circumference;

        total_distance_m += distance_moved;
        float speed_m_s = distance_moved / time_elapsed;
        last_speed_m_s = speed_m_s;

        return abs(speed_m_s);
    }

    float get_total_distance_m() {
        int64_t current_position = encoder.getCount();
        float rotations = (current_position - previous_position) / static_cast<float>(ppr);
        previous_position = current_position; // Update for the next call

        float distance_moved = rotations * (PI * (wheel_diameter_mm / 1000.0)) / gear_ratio;
        total_distance_m += abs(distance_moved);

        return abs(total_distance_m);
    }

    float get_acceleration_m_s2() {
        // Store the current timestamp
        unsigned long current_time = millis();

        // Calculate the time elapsed since the last speed measurement
        float time_elapsed = (current_time - previous_time) / 1000.0;

        // If there was no time elapsed, return 0 to avoid division by zero
        if (time_elapsed == 0) return 0;

        // Get the current speed
        float current_speed_m_s = get_speed_m_s(); // This also updates previous_time and previous_position

        // Calculate the acceleration
        float acceleration_m_s2 = (current_speed_m_s - last_speed_m_s) / time_elapsed;

        // Update the last speed measurement
        last_speed_m_s = current_speed_m_s;

        // Return the absolute value of acceleration
        return acceleration_m_s2;
    }

    void reset_distance() {
        total_distance_m = 0;
    }

    ESP32Encoder encoder;
};

#endif // DEKKTESTER_WHEELSPEEDENCODER_HPP