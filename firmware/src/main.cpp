/*
 * Copyright (c) 2025 GentleCare Corporation. All rights reserved.
 *
 * This source code and the accompanying materials are the confidential and
 * proprietary information of GentleCare Corporation. Unauthorized copying or
 * distribution of this file, via any medium, is strictly prohibited without
 * the prior written permission of GentleCare Corporation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>
#include "dimmer.h"
#include "servo.h"
#include "pressure_sensor.h"
#include "pressure_controller.h"
#include "motor_controller.h"

static int64_t print_timer = 0;

static float SERVO_ANGLE1 = 7.0;
static float SERVO_ANGLE2 = SERVO_ANGLE1 + 90.0;

void setup()
{
    Serial.begin(BAUD_RATE);
    while (!Serial);

    dimmer_initialize(DIMMER_ZC_PIN, DIMMER_PSM_PIN);
    dimmer_set_power(0.0);
    
    servo_initialize(SERVO_PWM_PIN);
    servo_set_angle(SERVO_ANGLE1);

    pressure_sensor_initialize(PRESSURE_SENSOR_ADC_PIN);

    if (!motor_controller_initialize(MOTOR_CONTROLLER_RX_PIN, MOTOR_CONTROLLER_TX_PIN)) {
        Serial.println("Failed to initialize motor controller.");
        abort();
    }

    print_timer = micros();
}

void loop()
{
    if (Serial.available() >= 2) {
        int8_t command = Serial.read();
        Serial.printf("Received command: %c\n", command);
        if (command == 'd') {
            int8_t encoded_power = Serial.read();
            float power = encoded_power >= '1' && encoded_power <= '9' ? 0.3 * (encoded_power - '1') / ('9' - '1') + 0.2 : 0.0;
            dimmer_set_power(power);
            Serial.printf("Setting dimmer power to: %.2f\n", power);
        }

        if (command == 's') {
            int8_t encoded_angle = Serial.read();
            if (encoded_angle == '1' || encoded_angle == '2') {
                servo_set_angle(encoded_angle == '1' ? SERVO_ANGLE1 : SERVO_ANGLE2);
                Serial.printf("Setting servo mode to: %c\n", encoded_angle);
            }
        }

        if (command == 'a') {
            int8_t encoded_pressure = Serial.read();
            float pressure = (encoded_pressure - '0') / 10.0 * 2.0;
            pressure_controller_set_reference(pressure);
            Serial.printf("Setting pressure to: %.2f\n", pressure);
        }

        if (command == 'm') {
            int8_t encoded_velocity = Serial.read();
            float velocity = (encoded_velocity - '0') / 10.0 * 50.0 - 25.0;
            motor_controller_set_velocity(velocity);
            Serial.printf("Setting motor velocity to: %.2f\n", velocity);
        }
    }

    dimmer_update();
    servo_update();

    bool calibrate = dimmer_get_power() == 0.0;
    float pressure = pressure_sensor_read(calibrate);
    float pressure_derivative = pressure_sensor_read_derivative();
    pressure_controller_update(pressure, pressure_derivative);

#if DEBUG_MODE
    if (micros() - print_timer >= 100000) {
        Serial.printf(">Air Pressure (PSI):%.4f\n", pressure);
        Serial.printf(">Pressure Derivative (PSI/s):%.4f\n", pressure_derivative);
        Serial.printf(">Voltage Dimmer (%%):%.2f\n", dimmer_get_power() * 100.0);
        Serial.printf(">Servo Angle (deg):%.2f\n", servo_get_angle());
        Serial.printf(">Motor Position (rotations):%.2f\n", motor_controller_get_position());
        Serial.printf(">Motor Velocity (rotations/s):%.2f\n", motor_controller_get_velocity());
        print_timer = micros();
    }
#endif
}