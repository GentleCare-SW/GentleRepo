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
#include <ESP32Encoder.h>
#include <AceButton.h>
#include "common/protocol.h"

static BluetoothSerial SerialBT;

static ESP32Encoder knob1;
static ESP32Encoder knob2;
static ace_button::AceButton button1(BUTTON1_PIN);
static ace_button::AceButton button2(BUTTON2_PIN);

static int64_t last_knob1_position = 0;
static int64_t last_knob2_position = 0;

static float air_pressure_reference = 0.0;
static float motor_velocity = 0.0;
static int32_t chamber = 0;

static int64_t monitor_timer = 0;

static void on_button_event(ace_button::AceButton *button, uint8_t event_type, uint8_t button_state)
{
    if (event_type != ace_button::AceButton::kEventReleased)
        return;

    if (button->getPin() == BUTTON1_PIN) {
        chamber = 1 - chamber;
        write_message(&SerialBT, COMMAND_CODE_SET_VALVE2_PERCENTAGE, chamber);
        wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS);
    } else if (button->getPin() == BUTTON2_PIN) {
        motor_velocity = 0.0;
        air_pressure_reference = 0.0;
        write_message(&SerialBT, COMMAND_CODE_SET_MOTOR_VELOCITY, motor_velocity);
        write_message(&SerialBT, COMMAND_CODE_SET_PRESSURE_REFERENCE, air_pressure_reference);
        wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS);
        wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS);
    }
}

void setup()
{
    Serial.begin(BAUD_RATE);
    while (!Serial);

    SerialBT.begin("GentleRemote", true);
    while (!SerialBT);

    knob1.attachFullQuad(KNOB1_DT_PIN, KNOB1_CLK_PIN);
    knob1.setCount(0);
    knob2.attachFullQuad(KNOB2_DT_PIN, KNOB2_CLK_PIN);
    knob2.setCount(0);
    button1.getButtonConfig()->setEventHandler(on_button_event);
    button2.getButtonConfig()->setEventHandler(on_button_event);
}

void loop()
{
    while (!SerialBT.connected()) {
        motor_velocity = 0.0;
        air_pressure_reference = 0.0;

        Serial.printf("Trying to connect to device...\n");
        if (!SerialBT.connect("GentleWedge"))
            Serial.printf("Could not find device, retrying...\n");
        else
            Serial.printf("Connected to device!\n");
    }

    while (SerialBT.available())
        SerialBT.read();

    int64_t knob1_position = knob1.getCount();
    if (knob1_position != last_knob1_position) {
        air_pressure_reference += (knob1_position - last_knob1_position) * 0.1;
        air_pressure_reference = constrain(air_pressure_reference, 0.0, 10.0);
        write_message(&SerialBT, COMMAND_CODE_SET_PRESSURE_REFERENCE, air_pressure_reference);
        wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS);

        last_knob1_position = knob1_position;
    }

    int64_t knob2_position = knob2.getCount();
    if (knob2_position != last_knob2_position) {
        motor_velocity += (knob2_position - last_knob2_position) * -0.5;
        motor_velocity = constrain(motor_velocity, -25.0, 25.0);
        write_message(&SerialBT, COMMAND_CODE_SET_MOTOR_VELOCITY, motor_velocity);
        wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS);

        last_knob2_position = knob2_position;
    }
    
    button1.check();
    button2.check();

    if (millis() - monitor_timer >= 100) {
        int response_code = 0;
        float value = 0.0;

        write_message(&SerialBT, COMMAND_CODE_GET_MOTOR_TORQUE);
        wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS, &response_code, &value);
        if (abs(value) > MAXIMUM_TORQUE) {
            motor_velocity = 0.0;
            air_pressure_reference = 0.0;
            write_message(&SerialBT, COMMAND_CODE_SET_MOTOR_VELOCITY, motor_velocity);
            write_message(&SerialBT, COMMAND_CODE_SET_PRESSURE_REFERENCE, air_pressure_reference);
            wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS);
            wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS);
        }

#if DEBUG_MODE
        write_message(&SerialBT, COMMAND_CODE_GET_VALVE1_PERCENTAGE);
        write_message(&SerialBT, COMMAND_CODE_GET_PRESSURE1);
        write_message(&SerialBT, COMMAND_CODE_GET_PRESSURE2);
        write_message(&SerialBT, COMMAND_CODE_GET_PRESSURE_REFERENCE);
        write_message(&SerialBT, COMMAND_CODE_GET_MOTOR_VELOCITY);
        write_message(&SerialBT, COMMAND_CODE_GET_MOTOR_POSITION);
        write_message(&SerialBT, COMMAND_CODE_GET_MOTOR_TORQUE);

        if (wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS, &response_code, &value))
            Serial.printf(">Valve Percentage (%%): %.2f\n", value * 100.0);
        if (wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS, &response_code, &value))
            Serial.printf(">Valve Pressure (PSI): %.2f\n", value);
        if (wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS, &response_code, &value))
            Serial.printf(">Vessel Pressure (PSI): %.2f\n", value);
        if (wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS, &response_code, &value))
            Serial.printf(">Pressure Reference (PSI): %.2f\n", value);
        if (wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS, &response_code, &value))
            Serial.printf(">Motor Velocity (rotations/s): %.2f\n", value);
        if (wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS, &response_code, &value))
            Serial.printf(">Motor Position (rotations): %.2f\n", value);
        if (wait_for_response(&SerialBT, RESPONSE_TIMEOUT_MS, &response_code, &value))
            Serial.printf(">Motor Torqe (Nm): %.2f\n", value);
#endif

        monitor_timer = millis();
    }
}