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

static float dimmer_level = 0.0;
static float motor_velocity = 0.0;
static int32_t chamber = 0;

static void on_button_event(ace_button::AceButton *button, uint8_t event_type, uint8_t button_state)
{
    if (event_type != ace_button::AceButton::kEventReleased)
        return;

    if (button->getPin() == BUTTON1_PIN) {
        chamber = 1 - chamber;

        uint8_t message[MESSAGE_LENGTH] = { 0 };
        message[0] = COMMAND_CODE_SET_CHAMBER;
        message_content_t content = { .int_value = chamber };
        memcpy(&message[1], &content, sizeof(content));
        SerialBT.write(message, MESSAGE_LENGTH);
        Serial.printf("Chamber: %d\n", chamber);
    } else if (button->getPin() == BUTTON2_PIN) {
        dimmer_level = 0.0;
        motor_velocity = 0.0;

        uint8_t message[MESSAGE_LENGTH] = { 0 };
        message[0] = COMMAND_CODE_SET_DIMMER_LEVEL;
        message_content_t content = { .float_value = dimmer_level };
        memcpy(&message[1], &content, sizeof(content));
        SerialBT.write(message, MESSAGE_LENGTH);
        message[0] = COMMAND_CODE_SET_MOTOR_VELOCITY;
        content.float_value = motor_velocity;
        memcpy(&message[1], &content, sizeof(content));
        SerialBT.write(message, MESSAGE_LENGTH);
        Serial.printf("Reset dimmer and motor velocity to 0.\n");
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
        dimmer_level = 0.0;
        motor_velocity = 0.0;

        Serial.printf("Trying to connect to device...\n");
        if (!SerialBT.connect("GentleWedge"))
            Serial.printf("Could not find device, retrying...\n");
        else
            Serial.printf("Connected to device!\n");
    }

    int64_t knob1_position = knob1.getCount();
    if (knob1_position != last_knob1_position) {
        dimmer_level += (knob1_position - last_knob1_position) * 0.005;
        dimmer_level = constrain(dimmer_level, 0.0, 1.0);

        uint8_t message[MESSAGE_LENGTH] = { 0 };
        message[0] = COMMAND_CODE_SET_DIMMER_LEVEL;
        message_content_t content = { .float_value = dimmer_level };
        memcpy(&message[1], &content, sizeof(content));
        SerialBT.write(message, MESSAGE_LENGTH);

        Serial.printf("Dimmer Level: %.2f\n", dimmer_level);

        last_knob1_position = knob1_position;
    }

    int64_t knob2_position = knob2.getCount();
    if (knob2_position != last_knob2_position) {
        motor_velocity += (knob2_position - last_knob2_position) * 0.5;
        motor_velocity = constrain(motor_velocity, -25.0, 25.0);

        uint8_t message[MESSAGE_LENGTH] = { 0 };
        message[0] = COMMAND_CODE_SET_MOTOR_VELOCITY;
        message_content_t content = { .float_value = motor_velocity };
        memcpy(&message[1], &content, sizeof(content));
        SerialBT.write(message, MESSAGE_LENGTH);
        Serial.printf("Motor Velocity: %.2f\n", motor_velocity);

        last_knob2_position = knob2_position;
    }

    while (SerialBT.available())
        SerialBT.read();
    
    button1.check();
    button2.check();
}