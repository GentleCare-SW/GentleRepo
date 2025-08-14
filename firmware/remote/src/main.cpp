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
#include <ESP32Encoder.h>
#include <AceButton.h>
#include "remote.h"

static Remote remote;

static ESP32Encoder knob1;
static ESP32Encoder knob2;
static ace_button::AceButton button1(BUTTON1_PIN);
static ace_button::AceButton button2(BUTTON2_PIN);

static int64_t last_knob1_position = 0;
static int64_t last_knob2_position = 0;

static float voltage_percentage = 0.0;
static float air_pressure_reference = 0.0;
static float motor_velocity = 0.0;

static int64_t monitor_timer = 0;

static void on_button_event(ace_button::AceButton *button, uint8_t event_type, uint8_t button_state)
{
    if (event_type != ace_button::AceButton::kEventReleased)
        return;

    if (button->getPin() == BUTTON1_PIN) {
        voltage_percentage = 0.0;
        remote.set_voltage_percentage(voltage_percentage);
    } else if (button->getPin() == BUTTON2_PIN) {
        motor_velocity = 0.0;
        remote.set_motor_velocity(motor_velocity);
    }
}

void setup()
{
    Serial.begin(BAUD_RATE);
    while (!Serial);

    knob1.attachFullQuad(KNOB1_DT_PIN, KNOB1_CLK_PIN);
    knob1.setCount(0);
    knob2.attachFullQuad(KNOB2_DT_PIN, KNOB2_CLK_PIN);
    knob2.setCount(0);
    button1.getButtonConfig()->setEventHandler(on_button_event);
    button2.getButtonConfig()->setEventHandler(on_button_event);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    remote.start();
}

void loop()
{
    remote.update();

    int64_t knob1_position = knob1.getCount();
    if (knob1_position != last_knob1_position) {
#if OPEN_LOOP_PRESSURE_CONTROL
        voltage_percentage += (knob1_position - last_knob1_position) * 0.01;
        voltage_percentage = constrain(voltage_percentage, 0.0, 1.0);
        remote.set_voltage_percentage(voltage_percentage);
#else
#endif
        last_knob1_position = knob1_position;
    }

    int64_t knob2_position = knob2.getCount();
    if (knob2_position != last_knob2_position) {
        motor_velocity += (knob2_position - last_knob2_position) * 0.05;
        motor_velocity = constrain(motor_velocity, -2.0, 2.0);
        remote.set_motor_velocity(motor_velocity);
        last_knob2_position = knob2_position;
    }
    
    button1.check();
    button2.check();

    if (millis() - monitor_timer >= 100) {
        float torque = remote.get_motor_torque();
        if (abs(torque) > MAXIMUM_TORQUE) {
            motor_velocity = 0.0;
            remote.set_motor_velocity(motor_velocity);
        }

        float pressure = remote.get_pressure();
        digitalWrite(BUZZER_PIN, pressure > 3.0 ? HIGH : LOW);

#if DEBUG_MODE
        Serial.printf(">Pressure (PSI): %f\n", pressure);
        Serial.printf(">Motor Position (radians): %f\n", remote.get_motor_position());
        Serial.printf(">Motor Velocity (radians/s): %f\n", remote.get_motor_velocity());
        Serial.printf(">Motor Torque (Nm): %f\n", torque);
        Serial.printf(">Voltage Percentage (%%): %f\n", remote.get_voltage_percentage());
        Serial.printf(">Time (ms): %lu\n", millis() - monitor_timer);
#endif

        monitor_timer = millis();
    }
}