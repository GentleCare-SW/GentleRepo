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

static int64_t print_timer = 0;

void setup()
{
    Serial.begin(BAUD_RATE);
    dimmer_initialize(DIMMER_ZC_PIN, DIMMER_PSM_PIN);
    dimmer_set_power(0.0);
    
    servo_initialize(SERVO_PWM_PIN);
    servo_set_angle(0.0);

    pressure_sensor_initialize(PRESSURE_SENSOR_ADC_PIN);

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
            float angle = (encoded_angle - '0') / 9.0 * 180.0;
            servo_set_angle(angle);
            Serial.printf("Setting servo angle to: %.2f\n", angle);
        }

        if (command == 'a') {
            int8_t encoded_pressure = Serial.read();
            float pressure = (encoded_pressure - '0') / 10.0 * 2.0;
            pressure_controller_set_reference(pressure);
            Serial.printf("Setting pressure to: %.2f\n", pressure);
        }
    }

    dimmer_update();
    servo_update();

    float pressure = pressure_sensor_read();
    float pressure_derivative = pressure_sensor_read_derivative();
    pressure_controller_update(pressure, pressure_derivative);

    if (micros() - print_timer >= 100000) {
        float dimmer_power = dimmer_get_power();
        Serial.printf(">Air Pressure (PSI):%.4f\n>Voltage Dimmer (%%):%.2f\n", pressure, dimmer_power * 100.0);
        print_timer = micros();
    }
}