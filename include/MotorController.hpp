#ifndef DEKKTESTER_MOTORCONTROLLER_HPP
#define DEKKTESTER_MOTORCONTROLLER_HPP

#include <Arduino.h>

class MotorController {
private:
    int pwm_pin;
    float max_voltage;
    const int freq = 5000; // Frequency for PWM signal
    const int ledChannel = 0; // Use channel 0 for PWM
    const int resolution = 8; // 8-bit resolution, 0-255
    float current_speed_percentage = 0; // Tracks the last speed percentage set
    bool is_motor_on = false; // Tracks the motor state

public:
    MotorController(int pin, float voltage) : pwm_pin(pin), max_voltage(voltage) {
        // Configure PWM for the motor control pin
        ledcSetup(ledChannel, freq, resolution);
        ledcAttachPin(pwm_pin, ledChannel);
    }

    void set_voltage(float voltage) {
        max_voltage = voltage;
    }

    void speed(float target_percentage, unsigned int duration_seconds) {
        unsigned long end_time = millis() + (duration_seconds * 1000);
        float initial_percentage = current_speed_percentage;

        while (millis() < end_time) {
            unsigned long elapsed_time = millis() - (end_time - duration_seconds * 1000);
            float progress = static_cast<float>(elapsed_time) / (duration_seconds * 1000.0);
            float new_percentage = initial_percentage + (progress * (target_percentage - initial_percentage));
            set_speed(new_percentage);
            delay(10); // Small delay to ensure smoother transition
        }
        current_speed_percentage = target_percentage; // Update the current speed for next adjustment
        set_speed(target_percentage); // Ensure target speed is set at the end
    }

    void set_speed(float percentage) {
        if (percentage < 0) percentage = 0;
        if (percentage > 100) percentage = 100;
        // Calculate PWM value considering the voltage scaling
        int pwmValue = static_cast<int>((percentage / 100.0) * 255.0 * (max_voltage / 12.0));
        ledcWrite(ledChannel, pwmValue);
        // Update motor state
        is_motor_on = (percentage > 0);
    }

    bool is_motor_running() const {
        return is_motor_on;
    }
};
#endif //DEKKTESTER_MOTORCONTROLLER_HPP