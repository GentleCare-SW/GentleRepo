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
#include <BluetoothSerial.h>
#include "dimmer.h"
#include "servo.h"
#include "pressure_sensor.h"
#include "pressure_controller.h"
#include "motor_controller.h"

static BluetoothSerial SerialBT;

static int64_t print_timer = 0;

static float SERVO_ANGLE1 = 7.0;
static float SERVO_ANGLE2 = SERVO_ANGLE1 + 90.0;

void setup()
{
    Serial.begin(BAUD_RATE);
    while (!Serial); 

    if (!SerialBT.begin("GentleWedge"))
        Serial.println("Failed to initialize Bluetooth Serial.");
    while (!SerialBT);

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
    if (SerialBT.available() >= 2) {
        int8_t command = SerialBT.read();

        if (command == 'd') {
            uint8_t encoded_power = SerialBT.read();
            float power = encoded_power / 255.0;
            dimmer_set_power(power);
        }

        if (command == 'c') {
            uint8_t chamber = SerialBT.read();
            if (chamber == '1')
                servo_set_angle(SERVO_ANGLE1);
            else if (chamber == '2')
                servo_set_angle(SERVO_ANGLE2);
        }

        if (command == 's') {
            uint8_t encoded_angle = SerialBT.read();
            float angle = encoded_angle / 255.0 * 180.0;
            servo_set_angle(angle);
        }

        if (command == 'a') {
            uint8_t encoded_pressure = SerialBT.read();
            float pressure = encoded_pressure / 255.0 * 5.0;
            pressure_controller_set_reference(pressure);
        }

        if (command == 'v') {
            uint8_t encoded_velocity = SerialBT.read();
            float velocity = (encoded_velocity / 255.0) * 50.0 - 25.0;
            motor_controller_set_velocity(velocity);
        }
    }

    dimmer_update();
    servo_update();

    bool calibrate = dimmer_get_power() == 0.0;
    float pressure = pressure_sensor_read(calibrate);
    float pressure_derivative = pressure_sensor_read_derivative();
    pressure_controller_update(pressure, pressure_derivative);

    if (micros() - print_timer >= 100000) {
#if DEBUG_MODE
        Serial.printf(">Air Pressure (PSI):%.4f\n", pressure);
        Serial.printf(">Pressure Derivative (PSI/s):%.4f\n", pressure_derivative);
        Serial.printf(">Voltage Dimmer (%%):%.2f\n", dimmer_get_power() * 100.0);
        Serial.printf(">Servo Angle (deg):%.2f\n", servo_get_angle());
        Serial.printf(">Motor Position (rotations):%.2f\n", motor_controller_get_position());
        Serial.printf(">Motor Velocity (rotations/s):%.2f\n", motor_controller_get_velocity());
        Serial.printf(">Motor Current (A):%.2f\n", motor_controller_get_current());
#endif
        SerialBT.printf("pressure:%f\n", pressure);
        SerialBT.printf("dimmer_power:%f\n", dimmer_get_power());
        SerialBT.printf("servo_angle:%f\n", servo_get_angle());
        SerialBT.printf("motor_position:%f\n", motor_controller_get_position());
        SerialBT.printf("motor_velocity:%f\n", motor_controller_get_velocity());
        SerialBT.printf("motor_current:%f\n", motor_controller_get_current());
        print_timer = micros();
    }
}