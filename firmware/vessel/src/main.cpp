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
#include "pressure_sensor.h"
#include "pressure_controller.h"
#include "motor_controller.h"
#include "valve.h"
#include "common/protocol.h"

static BluetoothSerial SerialBT;

static valve_t pressure_valve;
static valve_t wedge_valve;

static pressure_sensor_t valve_sensor;
static pressure_sensor_t vessel_sensor;
static pressure_sensor_t wedge_sensor;

static pressure_controller_t pressure_controller;

static void process_commands()
{
    int command;
    float value;
    while (read_message(&SerialBT, &command, &value)) {
        switch (command) {
        case COMMAND_CODE_SET_VALVE1_PERCENTAGE:
            valve_set_percentage(&pressure_valve, value);
            write_message(&SerialBT, RESPONSE_CODE_OK);
            break;
        case COMMAND_CODE_SET_PRESSURE_REFERENCE:
            pressure_controller_set_reference(&pressure_controller, value);
            write_message(&SerialBT, RESPONSE_CODE_OK);
            break;
        case COMMAND_CODE_GET_VALVE1_PERCENTAGE:
            write_message(&SerialBT, RESPONSE_CODE_OK, valve_get_percentage(&pressure_valve));
            break;
        case COMMAND_CODE_GET_PRESSURE1:
            write_message(&SerialBT, RESPONSE_CODE_OK, pressure_sensor_get_pressure(&valve_sensor));
            break;
        case COMMAND_CODE_GET_PRESSURE2:
            write_message(&SerialBT, RESPONSE_CODE_OK, pressure_sensor_get_pressure(&vessel_sensor));
            break;
        case COMMAND_CODE_GET_PRESSURE_REFERENCE:
            write_message(&SerialBT, RESPONSE_CODE_OK, pressure_controller_get_reference(&pressure_controller));
            break;
#if ENABLE_MOTOR_CONTROLLER
        case COMMAND_CODE_SET_MOTOR_VELOCITY:
            motor_controller_set_velocity(value);
            write_message(&SerialBT, RESPONSE_CODE_OK);
            break;
        case COMMAND_CODE_SET_MOTOR_TORQUE:
            motor_controller_set_torque(value);
            write_message(&SerialBT, RESPONSE_CODE_OK);
            break;
        case COMMAND_CODE_GET_MOTOR_VELOCITY:
            write_message(&SerialBT, RESPONSE_CODE_OK, motor_controller_get_velocity());
            break;
        case COMMAND_CODE_GET_MOTOR_POSITION:
            write_message(&SerialBT, RESPONSE_CODE_OK, motor_controller_get_position());
            break;
        case COMMAND_CODE_GET_MOTOR_TORQUE:
            write_message(&SerialBT, RESPONSE_CODE_OK, motor_controller_get_torque());
            break;
#endif
        default:
            write_message(&SerialBT, RESPONSE_CODE_ERROR);
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

    valve_initialize(&pressure_valve, DAC1);
    valve_set_percentage(&pressure_valve, 0.0);

    pressure_sensor_initialize(&valve_sensor, VALVE_SENSOR_ADC_PIN, VALVE_SENSOR_CONSTANT);
    pressure_sensor_initialize(&vessel_sensor, VESSEL_SENSOR_ADC_PIN, VESSEL_SENSOR_CONSTANT);

    pressure_controller_initialize(&pressure_controller, &pressure_valve, &valve_sensor);

#if ENABLE_MOTOR_CONTROLLER
    if (!motor_controller_initialize(&Serial1, MOTOR_CONTROLLER_RX_PIN, MOTOR_CONTROLLER_TX_PIN)) {
        Serial.println("Failed to initialize motor controller.");
        abort();
    }
#endif
}

void loop()
{
    process_commands();

    bool calibrate = valve_get_percentage(&pressure_valve) == 0.0;
    pressure_sensor_update(&valve_sensor, calibrate);
    pressure_sensor_update(&vessel_sensor, calibrate);
    pressure_controller_update(&pressure_controller);
}