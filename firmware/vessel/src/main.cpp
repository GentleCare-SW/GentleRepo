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
#include "common/protocol.h"

static BluetoothSerial SerialBT;

static void send_response(response_code_t response, message_content_t content = { 0 })
{
    uint8_t data[MESSAGE_LENGTH];
    data[0] = (uint8_t)response;
    memcpy(&data[1], &content, sizeof(content));
    SerialBT.write(data, MESSAGE_LENGTH);
}

static void process_commands()
{
    while (SerialBT.available() >= MESSAGE_LENGTH) {
        uint8_t command = SerialBT.read();

        message_content_t content;
        SerialBT.readBytes((uint8_t *)&content, sizeof(content));

        switch (command) {
        case COMMAND_CODE_SET_DIMMER_LEVEL:
            dimmer_set_power(content.float_value);
            send_response(RESPONSE_CODE_OK);
            break;
        case COMMAND_CODE_SET_CHAMBER:
            servo_set_angle(content.int_value == 0 ? CHAMBER1_SERVO_ANGLE : CHAMBER2_SERVO_ANGLE);
            send_response(RESPONSE_CODE_OK);
            break;
        case COMMAND_CODE_SET_SERVO_ANGLE:
            servo_set_angle(content.float_value);
            send_response(RESPONSE_CODE_OK);
            break;
        case COMMAND_CODE_SET_PRESSURE_REFERENCE:
            pressure_controller_set_reference(content.float_value);
            send_response(RESPONSE_CODE_OK);
            break;
        case COMMAND_CODE_SET_MOTOR_VELOCITY:
            motor_controller_set_velocity(content.float_value);
            send_response(RESPONSE_CODE_OK);
            break;
        case COMMAND_CODE_GET_DIMMER_LEVEL:
            send_response(RESPONSE_CODE_OK, (message_content_t){ .float_value = dimmer_get_power() });
            break;
        case COMMAND_CODE_GET_SERVO_ANGLE:
            send_response(RESPONSE_CODE_OK, (message_content_t){ .float_value = servo_get_angle() });
            break;
        case COMMAND_CODE_GET_PRESSURE:
            send_response(RESPONSE_CODE_OK, (message_content_t){ .float_value = pressure_sensor_get_pressure() });
            break;
        case COMMAND_CODE_GET_PRESSURE_REFERENCE:
            send_response(RESPONSE_CODE_OK, (message_content_t){ .float_value = pressure_controller_get_reference() });
            break;
        case COMMAND_CODE_GET_MOTOR_VELOCITY:
            send_response(RESPONSE_CODE_OK, (message_content_t){ .float_value = motor_controller_get_velocity() });
            break;
        case COMMAND_CODE_GET_MOTOR_POSITION:
            send_response(RESPONSE_CODE_OK, (message_content_t){ .float_value = motor_controller_get_position() });
            break;
        case COMMAND_CODE_GET_MOTOR_CURRENT:
            send_response(RESPONSE_CODE_OK, (message_content_t){ .float_value = motor_controller_get_current() });
            break;
        default:
            send_response(RESPONSE_CODE_ERROR);
            break;
        }
    }
}

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
    servo_set_angle(CHAMBER1_SERVO_ANGLE);

    pressure_sensor_initialize(PRESSURE_SENSOR_ADC_PIN);

    if (!motor_controller_initialize(MOTOR_CONTROLLER_RX_PIN, MOTOR_CONTROLLER_TX_PIN)) {
        Serial.println("Failed to initialize motor controller.");
        abort();
    }
}

void loop()
{
    process_commands();

    dimmer_update();
    servo_update();
    pressure_sensor_update(dimmer_get_power() == 0.0);

    pressure_controller_update(pressure_sensor_get_pressure(), pressure_sensor_get_derivative());
}