#ifndef DEKKTESTER_SWIVEL_ENCODER_HPP
#define DEKKTESTER_SWIVEL_ENCODER_HPP

#include <Arduino.h>
#include <ESP32Encoder.h>

class SwivelEncoder {
private:
    int64_t previous_position = 0;
    unsigned long previous_time = 0;
    float gear_ratio;
    int ppr;
    float distance_from_center_to_wheel_mm;
    float total_distance_m = 0;

public:
    SwivelEncoder(int pin_a, int pin_b, int ppr, float gear_ratio, float distance_from_center_to_wheel_mm)
            : ppr(ppr),
              gear_ratio(gear_ratio),
              distance_from_center_to_wheel_mm(distance_from_center_to_wheel_mm)
    {
        encoder.attachSingleEdge(pin_a, pin_b);
        previous_position = encoder.getCount();
        previous_time = millis();
    }

    void set_distance_from_center_to_wheel(float distance) {
        distance_from_center_to_wheel_mm = distance;
    }

    float get_total_distance_m() {
        int64_t current_position = encoder.getCount();
        float angle_radians = (current_position - previous_position) * 2 * PI / static_cast<float>(ppr);
        previous_position = current_position;

        float distance_moved = (distance_from_center_to_wheel_mm / 1000.0) * angle_radians / gear_ratio;
        total_distance_m += abs(distance_moved);

        return abs(total_distance_m);
    }

    void reset_distance() {
        encoder.clearCount();
        total_distance_m = 0;
    }

    ESP32Encoder encoder;
};

#endif // DEKKTESTER_SWIVEL_ENCODER_HPP
