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
    this->velocity_setpoint = 0.0;
    this->transferring = false;
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
                this->platform->set(AUTO_CONTROL_MODE_UUID, 1.0);
            } else if (i == (int)ButtonType::SERVO) {
                float prev_angle = this->platform->get(SERVO_ANGLE_UUID);
                if (SERVO_ANGLE2-prev_angle > prev_angle-SERVO_ANGLE1)
                    this->platform->set(SERVO_ANGLE_UUID, SERVO_ANGLE2);
                else
                    this->platform->set(SERVO_ANGLE_UUID, SERVO_ANGLE1);
            } else if (i == (int)ButtonType::CHAMBER) {
                int new_state = (int)(1.0 + this->platform->get(VALVE_STATE_UUID)) % 3;
                this->platform->set(VALVE_STATE_UUID, (float)new_state);
                // if (new_state == 2) {
                //     delay(10);
                //     this->platform->set(CENTRAL_DIMMER_UUID, 0.0);
                // }
            } else if (i == (int)ButtonType::TRANSFER) {
                this->platform->set(OUTER_DIMMER_UUID, 25.0);
                this->platform->set(CENTRAL_DIMMER_UUID, 120.0);
                this->transferring = !this->transferring;
            } else if (i == (int)ButtonType::STOP_AIR1) {
                this->platform->set(CENTRAL_DIMMER_UUID, 0.0, true);
                this->platform->set(PRESSURE_CONTROLLER_UUID, 0.0, true);
            } else if (i == (int)ButtonType::STOP_AIR2) {
                this->platform->set(OUTER_DIMMER_UUID, 0.0, true);
                //this->platform->set(PRESSURE_CONTROLLER_UUID, 0.0, true);
            } else if (i == (int)ButtonType::STOP_MOTOR) {
                this->velocity_setpoint = 0.0;
                this->platform->set(MOTOR_VELOCITY_UUID, 0.0, true);
                if (this->platform->get(MOTOR_ERROR_UUID) == 2.0){
                    Serial.println("Trying to recalibrate");
                    this->platform->set(MOTOR_ERROR_UUID, 4.0);
                }
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
            if (i==0){
                this->velocity_setpoint = value;
            }
            Serial.printf("Knob changed: %i, %.1f\n", knob_difference, value);
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
            //this->display->printf("CALIBRATION ERROR");
            //TODO: automated recalibration procedure?
            this->platform->set(CENTRAL_DIMMER_UUID, 40.0);
            this->display->printf("AUTOMATIC RECALIBRATION");
            delay(1500);
            this->platform->set(CENTRAL_DIMMER_UUID, 0.0);
            delay(1000);
            this->platform->set(MOTOR_ERROR_UUID, 4.0);
            break;
        case 3:
            this->display->printf("MOTOR CONTROL ERROR");
            break;
        // default:
        //     if (this->platform->get(PRESSURE_SENSOR_ERROR_UUID) != 0.0)
        //         //TODO: replace with jam detected
        //         this->display->printf("PRESSURE ERROR: %i\n", (int)this->platform->get(PRESSURE_SENSOR_ERROR_UUID));
    }
    
    #if DEVELOPER_SCREEN
        #if PLATFORM_TYPE == 0
            this->display->setCursor(0, 8);
            float current_angle = this->platform->get(SERVO_ANGLE_UUID);
            
            this->display->printf("Servo angle: %i\n", (int)current_angle);
            float valve_state = this->platform->get(VALVE_STATE_UUID);
            if (valve_state == 0.0)
                this->display->printf("Valve: HOLD \n");
            else if (valve_state == 1.0)
                this->display->printf("Valve: DRAIN \n");
            else if (valve_state == 2.0)
                this->display->printf("Valve: FILL \n");
            this->display->printf("Position: %.1f rev\n", this->platform->get(MOTOR_POSITION_UUID));
        #else
            this->display->setCursor(0, 16);
            this->display->printf("Position: %.1f rev\n", this->platform->get(MOTOR_POSITION_UUID));
        #endif
        this->display->printf("Vel: %.1f RPM\n", this->platform->get(MOTOR_VELOCITY_UUID));
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

    #if CLOSED_LOOP_PRESSURE_CONTROL
        this->display->printf("Pressure: %.2f PSI\n", this->platform->get(PRESSURE_CONTROLLER_UUID));
    #else
        #if PLATFORM_TYPE == 0
            this->display->printf("Voltage: %.1f V\n", this->platform->get(CENTRAL_DIMMER_UUID));
            float p1 = std::max((float)0.0, this->platform->get(PRESSURE_SENSOR_UUID));
            float p2 = std::max((float)0.0, this->platform->get(PRESSURE_SENSOR2_UUID));
            this->display->printf("PSI: %.2f | %.2f \n", p1, p2);
        #else
            this->display->printf("Voltage: %.1f, %.1f\n", this->platform->get(CENTRAL_DIMMER_UUID), this->platform->get(OUTER_DIMMER_UUID));
            this->display->printf("Pressure: %.2f PSI\n", std::max((float)0.0, this->platform->get(PRESSURE_SENSOR_UUID)));
        #endif
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
