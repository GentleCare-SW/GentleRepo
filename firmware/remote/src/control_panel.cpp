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

#include "control_panel.h"
#include "config.h"

ControlPanel::ControlPanel(RemotePlatform *platform, Adafruit_SSD1306 *display)
{
    this->platform = platform;
    this->display = display;
}

void ControlPanel::start(uint32_t button_pins[(int)ButtonType::COUNT], uint32_t knob_dt_pins[(int)KnobType::COUNT], uint32_t knob_clk_pins[(int)KnobType::COUNT])
{
    for (int i = 0; i < (int)ButtonType::COUNT; i++) {
        this->button_pins[i] = button_pins[i];
        this->button_pressed[i] = false;
        pinMode(button_pins[i], INPUT_PULLUP);
    }
    
    for (int i = 0; i < (int)KnobType::COUNT; i++) {
        this->knobs[i].attachFullQuad(knob_dt_pins[i], knob_clk_pins[i]);
        this->knobs[i].setCount(0);
        this->last_knob_positions[i] = 0;
    }
}

void ControlPanel::update_buttons()
{
    for (int i = 0; i < (int)ButtonType::COUNT; i++) {
        bool pressed = digitalRead(this->button_pins[i]) == LOW;

        if (!pressed && this->button_pressed[i]) {
            if (i == (int)ButtonType::STOP) {
                this->platform->set(MOTOR_VELOCITY_UUID, 0.0);
                this->platform->set(DIMMER_VOLTAGE_UUID, 0.0);
                this->platform->set(AUTO_CONTROL_MODE_UUID, 0.0);
                this->platform->set(PRESSURE_CONTROLLER_UUID, 0.0);
            } else if (i == (int)ButtonType::INVERT) {
                this->platform->set(AUTO_CONTROL_MODE_UUID, 3.0);
            } else if (i == (int)ButtonType::EVERT) {
                this->platform->set(AUTO_CONTROL_MODE_UUID, 1.0);
            } else if (i == (int)ButtonType::PAUSE) {
                float mode = this->platform->get(AUTO_CONTROL_MODE_UUID);
                if (mode == 1.0)
                    this->platform->set(AUTO_CONTROL_MODE_UUID, 2.0);
                else if (mode == 3.0)
                    this->platform->set(AUTO_CONTROL_MODE_UUID, 4.0);
                else if (mode == 2.0)
                    this->platform->set(AUTO_CONTROL_MODE_UUID, 1.0);
                else if (mode == 4.0)
                    this->platform->set(AUTO_CONTROL_MODE_UUID, 3.0);
            } else if (i == (int)ButtonType::CHAMBER) {
                this->platform->set(SERVO_CHAMBER_UUID, 1.0 - this->platform->get(SERVO_CHAMBER_UUID));
            } else if (i == (int)ButtonType::STOP_AIR) {
                this->platform->set(DIMMER_VOLTAGE_UUID, 0.0, true);
                this->platform->set(PRESSURE_CONTROLLER_UUID, 0.0, true);
            } else if (i == (int)ButtonType::STOP_MOTOR) {
                this->platform->set(MOTOR_VELOCITY_UUID, 0.0, true);
            }
        }

        this->button_pressed[i] = pressed;
    }
}

void ControlPanel::update_knobs()
{
    int64_t motor_knob_count = this->knobs[(int)KnobType::MOTOR].getCount();
    int64_t motor_knob_difference = motor_knob_count - this->last_knob_positions[(int)KnobType::MOTOR];
    if (motor_knob_difference != 0) {
        float velocity = this->platform->get(MOTOR_VELOCITY_UUID);
        velocity += motor_knob_difference * 1.0;
        velocity = constrain(velocity, -30.0, 30.0);
        this->platform->set(MOTOR_VELOCITY_UUID, velocity);
    }
    this->last_knob_positions[(int)KnobType::MOTOR] = motor_knob_count;

    int64_t air_knob_count = this->knobs[(int)KnobType::AIR].getCount();
    int64_t air_knob_difference = air_knob_count - this->last_knob_positions[(int)KnobType::AIR];
    if (air_knob_difference != 0) {
#if CLOSED_LOOP_PRESSURE_CONTROL
        float pressure_setpoint = this->platform->get(PRESSURE_CONTROLLER_UUID);
        pressure_setpoint += air_knob_difference * 0.005;
        pressure_setpoint = constrain(pressure_setpoint, 0.0, 1.0);
        this->platform->set(PRESSURE_CONTROLLER_UUID, pressure_setpoint);
#else
        float voltage = this->platform->get(DIMMER_VOLTAGE_UUID);
        voltage += air_knob_difference * 1.0;
        voltage = constrain(voltage, 0.0, 120.0);
        this->platform->set(DIMMER_VOLTAGE_UUID, voltage);
#endif
    }
    this->last_knob_positions[(int)KnobType::AIR] = air_knob_count;
}

void ControlPanel::update_display()
{
    this->display->clearDisplay();
    this->display->setCursor(0, 0);
    this->display->printf("Status: ");
    float mode = this->platform->get(AUTO_CONTROL_MODE_UUID);
    if (mode == 0.0)
        this->display->printf("Idle\n");
    else if (mode == 1.0)
        this->display->printf("Everting\n");
    else if (mode == 2.0)
        this->display->printf("Paused\n");
    else if (mode == 3.0)
        this->display->printf("Inverting\n");
    else if (mode == 4.0)
        this->display->printf("Paused\n");
    else
        this->display->printf("Unknown\n");
    
    switch ((int)this->platform->get(MOTOR_ERROR_UUID)){
        case 1:
            this->display->printf("MOTOR NOT RESPONDING");
            break;
        case 2:
            this->display->printf("MOTOR CALIBRATION ERROR");
            break;
        case 3:
            this->display->printf("MOTOR CONTROL ERROR");
            break;
        default:
            if (this->platform->get(PRESSURE_SENSOR_ERROR_UUID) != 0.0)
                this->display->printf("PRESSURE ERROR: %i\n", (int)this->platform->get(PRESSURE_SENSOR_ERROR_UUID));
    }
    

#if DEVELOPER_SCREEN
    this->display->setCursor(0, 16);
    this->display->printf("Chamber: %i\n", (int)this->platform->get(SERVO_CHAMBER_UUID));
    this->display->printf("Position: %.1f rev\n", this->platform->get(MOTOR_POSITION_UUID));
    this->display->printf("Velocity: %.1f RPM\n", this->platform->get(MOTOR_VELOCITY_UUID));
#else
    this->display->setCursor(0, 16);
    float progress = this->platform->get(AUTO_CONTROL_PROGRESS_UUID);
    this->display->printf("Progress: %.1f%%\n", progress * 100.0);
    this->display->fillRect(0, 24 + 1, (int16_t)(DISPLAY_WIDTH * progress), 8 - 2, SSD1306_WHITE);
    this->display->drawRect(0, 24 + 1, DISPLAY_WIDTH, 8 - 2, SSD1306_WHITE);
#endif

    this->display->setCursor(0, 40);
    this->display->printf("Torque: %.2f Nm\n", this->platform->get(MOTOR_TORQUE_UUID));
    this->display->printf("Voltage: %.1f V\n", this->platform->get(DIMMER_VOLTAGE_UUID));
#if CLOSED_LOOP_PRESSURE_CONTROL
    this->display->printf("Pressure: %.2f PSI\n", this->platform->get(PRESSURE_CONTROLLER_UUID));
#else
    this->display->printf("Pressure: %.2f PSI\n", this->platform->get(PRESSURE_SENSOR_UUID));
#endif

    this->display->display();
}

void ControlPanel::update()
{
    this->update_knobs();
    this->update_buttons();
    this->update_display();
}
