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

bool transferring = false;

ControlPanel::ControlPanel(RemotePlatform *platform, Adafruit_SSD1306 *display)
{
    this->platform = platform;
    this->display = display;
}

void ControlPanel::start(int32_t button_pins[(int)ButtonType::COUNT], uint32_t knob_dt_pins[(int)KnobType::COUNT], 
        uint32_t knob_clk_pins[(int)KnobType::COUNT], Knob knob_params[(int)KnobType::COUNT])
{
    for (int i = 0; i < (int)ButtonType::COUNT; i++) {
        this->button_pins[i] = button_pins[i];
        this->button_pressed[i] = false;
        pinMode(button_pins[i], INPUT_PULLUP);
    }
    
    for (int i = 0; i < (int)KnobType::COUNT; i++) {
        this->knobs[i].attachFullQuad(knob_dt_pins[i], knob_clk_pins[i]);
        this->knobs[i].setCount(0);
        this->current_knob_positions[i] = 0;
        this->last_knob_positions[i] = 0;
        this->knob_params[i] = knob_params[i];
    }

    pinMode(JOYSTICK_VRX_PIN, INPUT);
    
}

void ControlPanel::update_buttons()
{
    for (int i = 0; i < (int)ButtonType::COUNT; i++) {
        bool pressed = digitalRead(this->button_pins[i]) == LOW;

        if (!pressed && this->button_pressed[i]) {
            if (i == (int)ButtonType::STOP) {
                this->platform->set(MOTOR_VELOCITY_UUID, 0.0);
                this->platform->set(CENTRAL_DIMMER_UUID, 0.0);
                this->platform->set(OUTER_DIMMER_UUID, 0.0);
                this->platform->set(AUTO_CONTROL_MODE_UUID, 0.0);
                this->platform->set(PRESSURE_CONTROLLER_UUID, 0.0);
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
            } else if (i == (int)ButtonType::INVERT) {
                this->platform->set(AUTO_CONTROL_MODE_UUID, 3.0);
            } else if (i == (int)ButtonType::EVERT) {
                //this->platform->set(AUTO_CONTROL_MODE_UUID, 1.0);
                this->platform->set(JOYSTICK_UUID, -1.0);
                this->platform->set(OUTER_DIMMER_UUID, 30.0);
                this->platform->set(CENTRAL_DIMMER_UUID, 120.0);
                transferring = !transferring;
            } else if (i == (int)ButtonType::SERVO) {
                float prev_angle = this->platform->get(SERVO_ANGLE_UUID);
                if (SERVO_ANGLE2-prev_angle > prev_angle-SERVO_ANGLE1)
                    this->platform->set(SERVO_ANGLE_UUID, SERVO_ANGLE2);
                else
                    this->platform->set(SERVO_ANGLE_UUID, SERVO_ANGLE1);
            } else if (i == (int)ButtonType::CHAMBER) {
                float new_state = 1.0 - this->platform->get(ON_OFF_VALVE_UUID);
                this->platform->set(ON_OFF_VALVE_UUID, new_state);
                delay(10);
                this->platform->set(CENTRAL_DIMMER_UUID, 0.0);
            } else if (i == (int)ButtonType::STOP_AIR1) {
                this->platform->set(CENTRAL_DIMMER_UUID, 0.0, true);
                this->platform->set(PRESSURE_CONTROLLER_UUID, 0.0, true);
            } else if (i == (int)ButtonType::STOP_AIR2) {
                this->platform->set(OUTER_DIMMER_UUID, 0.0, true);
                //this->platform->set(PRESSURE_CONTROLLER_UUID, 0.0, true);
            } else if (i == (int)ButtonType::STOP_MOTOR) {
                this->platform->set(MOTOR_VELOCITY_UUID, 0.0, true);
            }
        }

        this->button_pressed[i] = pressed;
    }
}

void ControlPanel::update_knobs() 
{
    for (int i = 0; i < (int)KnobType::COUNT; i++) {
        this->current_knob_positions[i] = this->knobs[i].getCount();
        int64_t knob_difference = this->current_knob_positions[i] - this->last_knob_positions[i];
        
        if (knob_difference != 0) {
            Knob current_knob = this->knob_params[i];
            float value = this->platform->get(current_knob.UUID);
            value += knob_difference * current_knob.sensitivity;
            value = constrain(value, current_knob.lower_bound, current_knob.upper_bound);
            this->platform->set(current_knob.UUID, value);
        }
        this->last_knob_positions[i] = this->current_knob_positions[i];
    }
}

void ControlPanel::update_joystick()
{
    float x_value = (analogRead(JOYSTICK_VRX_PIN)/2048.0) - 1.0;
    if (x_value > 0.9) {
        this->platform->set(JOYSTICK_UUID, 1.0);
        transferring = false;
    } else if (x_value < -0.9) {
        this->platform->set(JOYSTICK_UUID, -1.0);
        transferring = false;
    } else if (!transferring)
        this->platform->set(JOYSTICK_UUID, 0.0);
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
            this->display->printf("CALIBRATION ERROR");
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
        
        #if PLATFORM_TYPE == 0
            this->display->printf("Servo angle: %i\n", (int)this->platform->get(SERVO_ANGLE_UUID)-5);
            this->display->printf("Valves: %.1f V | %.1f\n", this->platform->get(PROPORTIONAL_VALVE_UUID), this->platform->get(ON_OFF_VALVE_UUID));
        #else
            this->display->printf("Position: %.1f rev\n", this->platform->get(MOTOR_POSITION_UUID));
        #endif
        this->display->printf("Velocity: %.1f RPM\n", this->platform->get(MOTOR_VELOCITY_UUID));
    #else
        this->display->setCursor(0, 16);
        float progress = this->platform->get(AUTO_CONTROL_PROGRESS_UUID);
        this->display->printf("Progress: %.1f%%\n", progress * 100.0);
        this->display->fillRect(0, 24 + 1, (int16_t)(DISPLAY_WIDTH * progress), 8 - 2, SSD1306_WHITE);
        this->display->drawRect(0, 24 + 1, DISPLAY_WIDTH, 8 - 2, SSD1306_WHITE);
        this->display->setCursor(0, 32);
        this->display->printf("Velocity: %.1f RPM\n", this->platform->get(MOTOR_VELOCITY_UUID));
    #endif

    this->display->setCursor(0, 40);
    this->display->printf("Torque: %.2f Nm\n", this->platform->get(MOTOR_TORQUE_UUID));
    this->display->printf("Voltage: %.1f, %.1f\n", this->platform->get(CENTRAL_DIMMER_UUID), this->platform->get(OUTER_DIMMER_UUID));
    #if CLOSED_LOOP_PRESSURE_CONTROL
        this->display->printf("Pressure: %.2f PSI\n", this->platform->get(PRESSURE_CONTROLLER_UUID));
    #else
        this->display->printf("Pressure: %.2f PSI\n", std::max((float)0.0, this->platform->get(PRESSURE_SENSOR_UUID)));
    #endif

    this->display->display();
}

void ControlPanel::update()
{
    this->update_knobs();
    this->update_buttons();
    #if PLATFORM_TYPE==1
        this->update_joystick();
    #endif
    this->update_display();
}
