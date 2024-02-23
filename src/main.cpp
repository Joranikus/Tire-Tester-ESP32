#include <Arduino.h>
#include "Test.hpp"
#include "WheelSpeedEncoder.hpp"
#include "SwivelEncoder.hpp"
#include "MotorController.hpp"

/*WIKI:
 * ------------------------------------WheelSpeedEncoder------------------------------------
 * Måler direkte på hjulet, noe som gir deg optimale målinger hvis dekket ikke sklir.
 *
 * - hjulenkoder.get_total_distance_m() gir deg den optimale totale distansen hjulet har bevegd seg i m.
 * - hjulenkoder.get_speed_m_s() gir deg optimale farta i m/s.
 * - hjulenkoder.get_acceleration_m_s2() gir deg reelle akselerasjonen i m/s2.
 * - hjulenkoder.reset_distance() resetter den totale disansen.
 *
 * --------------------------------------SwivelEncoder--------------------------------------
 * Måler på svingarmen, noe som gir deg de reelle målingene.
 *
 * - svingarmenkoder.get_total_distance_m() gir deg den reelle totale distansen hjulet har bevegd seg i m.
 * - svingarmenkoder.get_speed_m_s() gir deg den reelle farta i m/s.
 * - svingarmenkoder.get_acceleration_m_s2() gir deg den reelle aksellerasjonen i m/s2.
 * - scingarmenkoder.reset_distance() resetter den totale disansen.
 *
 * -----------------------------------------------------------------------------------------
 * Med å sammenligne disse to målingene er det mulig å måle avvik mellom de, for eksempel
 * kan du da finne ut om dekket sklir mye. Da vil dekkmålingene være høyere en svingarm målingene.
*/

MotorController motor(18, 6.0);
WheelSpeedEncoder wheel_encoder(32, 33, 1000, 0.25, 74.0);
SwivelEncoder swivel_encoder(27, 14, 500, 2, 315.0);
//Test test(motor, wheel_encoder, swivel_encoder);

void setup() {
    Serial.begin(115200);
}

void loop() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Clean up the command string

        if (command.startsWith("set_motor_voltage ")) {
            float voltage = command.substring(18).toFloat();
            motor.set_voltage(voltage);
            Serial.println("Motor voltage set.");
        } else if (command.startsWith("set_wheel_diameter ")) {
            float diameter = command.substring(19).toFloat();
            wheel_encoder.set_wheel_diameter(diameter);
            Serial.println("Wheel diameter set.");
        } else if (command.startsWith("set_distance_center_to_wheel ")) {
            float distance = command.substring(29).toFloat();
            swivel_encoder.set_distance_from_center_to_wheel(distance);
            Serial.println("Distance from center to wheel set.");
        } else if (command.startsWith("run test")) {
            // Remove the command part to parse potential parameters
            command.remove(0, 8); // Remove "run_test "
            command.trim(); // Clean up the command string again

            float speed_percent = 50; // Default value
            unsigned int acceleration_time_s = 2; // Default value

            if (command.length() > 0) {
                // Parse the parameters
                int firstSpace = command.indexOf(' ');
                if (firstSpace != -1) {
                    speed_percent = command.substring(0, firstSpace).toFloat();
                    acceleration_time_s = command.substring(firstSpace + 1).toInt();
                } else {
                    // If there's no space, it means only one parameter was provided, which we assume to be speed_percent
                    speed_percent = command.toFloat();
                }
                Serial.println("Running test with custom parameters...");
            } else {
                Serial.println("Running test with default parameters...");
            }

            test.run_main_test(speed_percent, acceleration_time_s);
        } else {
            Serial.print("Unknown command received: ");
            Serial.println(command);
        }
    }
}

//----------------------------------------TESTS--------------------------------------------
/*
void loop() {
    // Print the values
    Serial.print("Distance Wheel: ");
    Serial.println(wheel_encoder.get_total_distance_m());
    Serial.print("Distance Swivel: ");
    Serial.println(swivel_encoder.get_total_distance_m());
    delay(300);
}
*/
//-----------------------------------------------------------------------------------------