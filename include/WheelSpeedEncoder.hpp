#ifndef DEKKTESTER_WHEELSPEEDENCODER_HPP
#define DEKKTESTER_WHEELSPEEDENCODER_HPP

#include <Arduino.h>
#include <ESP32Encoder.h>

class WheelSpeedEncoder {
private:
    int64_t previous_position = 0;
    int ppr;
    float gear_ratio;
    float wheel_diameter_mm;
    float total_distance_m = 0;

public:
    WheelSpeedEncoder(int pin_a, int pin_b, int ppr, float gear_ratio, float wheel_diameter_mm)
            : ppr(ppr),
              gear_ratio(gear_ratio),
              wheel_diameter_mm(wheel_diameter_mm)
    {
        encoder.attachSingleEdge(pin_a, pin_b);
        previous_position = encoder.getCount();
    }

    void set_wheel_diameter(float wheel_diameter) {
        wheel_diameter_mm = wheel_diameter;
    }

    float get_total_distance_m() {
        int64_t current_position = encoder.getCount();
        float rotations = (current_position - previous_position) / static_cast<float>(ppr);
        previous_position = current_position;

        float distance_moved = rotations * (PI * (wheel_diameter_mm / 1000.0)) / gear_ratio;
        total_distance_m += abs(distance_moved);

        return abs(total_distance_m);
    }

    void reset_distance() {
        total_distance_m = 0;
    }

    ESP32Encoder encoder;
};

#endif // DEKKTESTER_WHEELSPEEDENCODER_HPP
