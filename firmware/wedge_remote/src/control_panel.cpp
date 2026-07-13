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

static const float BLINK_INTERVAL = 400.0;
static const int MAX_SCORES[] = {4, 4, 4, 4, 4, 3};
static const std::string BRADEN_TITLES[] = {"Sensory", "Moisture", "Activity", "Mobility", "Nutrition", "Friction"};


ControlPanel::ControlPanel(RemotePlatform *platform, Adafruit_SSD1306 *display, PowerManagement *power)
{
    this->platform = platform;
    this->display = display;
    this->power = power;
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
    
    this->braden_score = {4, 4, 4, 4, 4, 3};
    this->prev_time = millis();
}

void ControlPanel::update_buttons()
{
    for (int i = 0; i < (int)ButtonType::COUNT; i++) {
        bool pressed = digitalRead(this->button_pins[i]) == LOW;
        //detects a press when user lets go
        if (!pressed && this->button_pressed[i]) {
            if (i == (int)ButtonType::STOP) {
                this->platform->set(MOTOR_VELOCITY_UUID, 0.0);
                this->platform->set(CENTRAL_DIMMER_UUID, 0.0);
                this->platform->set(OUTER_DIMMER_UUID, 0.0);
                this->state_before_stop = this->platform->get(AUTO_CONTROL_MODE_UUID);
                this->platform->set(AUTO_CONTROL_MODE_UUID, 0.0);
                this->platform->set(PRESSURE_CONTROLLER_UUID, 0.0);
                this->platform->set(JOYSTICK_UUID, 2.0);
            } else if (i == (int)ButtonType::PAUSE) {
                float mode = this->platform->get(AUTO_CONTROL_MODE_UUID);
                if (mode == 1.0)
                    this->platform->set(AUTO_CONTROL_MODE_UUID, 2.0);
                else if (mode == 2.0)
                    this->platform->set(AUTO_CONTROL_MODE_UUID, 1.0);
                else if (mode == 3.0)
                    this->platform->set(AUTO_CONTROL_MODE_UUID, 4.0);
                // else if (mode == 4.0)
                //     this->platform->set(AUTO_CONTROL_MODE_UUID, 3.0);
                else if (mode == 5.0)
                    this->platform->set(AUTO_CONTROL_MODE_UUID, 6.0);
                else if (mode == 6.0)
                    this->platform->set(AUTO_CONTROL_MODE_UUID, 5.0);
                delay(240);
                if (digitalRead(this->button_pins[i]) == LOW){
                    if (this->current_menu == MenuType::HOME)
                        this->current_menu = MenuType::BRADEN;
                    else if (this->current_menu == MenuType::BRADEN)
                        this->current_menu = MenuType::HOME;
                }
                    //this->braden_menu = !this->braden_menu;
            } else if (i == (int)ButtonType::PLAY) {
                if (this->current_menu != MenuType::HOME)
                    this->editing = !this->editing; 
                else {
                    float mode = this->platform->get(AUTO_CONTROL_MODE_UUID);
                    if (mode == 0.0 && this->state_before_stop != 0.0) {
                        float prev_state = this->state_before_stop;
                        if (prev_state == 2.0)
                            prev_state = 1.0;
                        else if (prev_state == 6.0)
                            prev_state = 5.0;
                        this->platform->set(AUTO_CONTROL_MODE_UUID, prev_state);
                        this->state_before_stop = 0.0; }
                    else if (mode == 0.0)
                        this->platform->set(AUTO_CONTROL_MODE_UUID, 1.0);
                    else if (mode == 2.0)
                        this->platform->set(AUTO_CONTROL_MODE_UUID, 1.0);
                    else if (mode == 6.0)
                        this->platform->set(AUTO_CONTROL_MODE_UUID, 5.0);
                }
            } else if (i == (int)ButtonType::INVERT) {
                if (this->current_menu == MenuType::BRADEN){
                    if (this->editing) {
                        int new_score = this->braden_score[this->currently_selected]-1;
                        new_score = constrain(new_score, 1, MAX_SCORES[this->currently_selected]);
                        this->braden_score[this->currently_selected] = new_score;
                    } else
                        this->currently_selected++;
                } else
                    this->platform->set(AUTO_CONTROL_MODE_UUID, 5.0);
            } else if (i == (int)ButtonType::EVERT) {
                if (this->current_menu == MenuType::BRADEN){
                    if (this->editing){
                        int new_score = this->braden_score[this->currently_selected]+1;
                        new_score = constrain(new_score, 1, MAX_SCORES[this->currently_selected]);
                        this->braden_score[this->currently_selected] = new_score;
                    } else
                        this->currently_selected--;
                } else
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
            } else if (i == (int)ButtonType::TRANSFER) {
                this->platform->set(AUTO_CONTROL_MODE_UUID, 3.0);
                // float voltage_change = (int)(this->platform->get(CENTRAL_DIMMER_UUID) - 80) % 40;
                // this->platform->set(CENTRAL_DIMMER_UUID, this->platform->get(CENTRAL_DIMMER_UUID) + voltage_change);
            } else if (i == (int)ButtonType::STOP_AIR1) {
                this->platform->set(CENTRAL_DIMMER_UUID, 0.0, true);
                this->platform->set(PRESSURE_CONTROLLER_UUID, 0.0, true);
            } else if (i == (int)ButtonType::STOP_AIR2) {
                this->platform->set(OUTER_DIMMER_UUID, 0.0, true);
            } else if (i == (int)ButtonType::STOP_MOTOR) {
                this->velocity_setpoint = 0.0;
                this->platform->set(MOTOR_VELOCITY_UUID, 0.0, true);
                if (this->platform->get(MOTOR_ERROR_UUID) == 2.0){
                    //Serial.println("Trying to recalibrate");
                    this->platform->set(MOTOR_ERROR_UUID, 4.0);
                }
            }
        }
        this->button_pressed[i] = pressed;
    }
    //this->currently_selected = constrain(this->currently_selected, 0, 5);
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
        delay(25);
    } else if (x_value < -0.9) {
        this->platform->set(JOYSTICK_UUID, -1.0);
        delay(25);
    } else if (this->platform->get(JOYSTICK_UUID) != 2.0)
        this->platform->set(JOYSTICK_UUID, 0.0);
}

void ControlPanel::update_display()
{
    this->display->clearDisplay();
    this->display->setCursor(0, 0);
    this->display->printf("Status: ");
    #if DEVELOPER_SCREEN == 0
        this->display->print("\n");
    #endif
    float mode = this->platform->get(AUTO_CONTROL_MODE_UUID);
    //TODO: make this a switch?
    if (mode == 0.0)
        this->display->printf("Idle\n");
    else if (mode == 1.0)
        this->display->printf("Extending\n");
    else if (mode == 2.0)
        this->display->printf("Paused\n");
    else if (mode == 3.0) {
        this->display->printf("Inflating\n");
    } else if (mode == 4.0) {
        long timer = this->platform->get(TIMER_UUID) * 0.001;
        int secs = timer%60;
        int mins = (int)((timer - secs)/60);
        this->display->printf("Holding %i:%02i", mins, secs);
    } else if (mode == 5.0)
        this->display->printf("Retracting\n");
    else if (mode == 6.0)
        this->display->printf("Paused\n");
    else
        this->display->printf("Unknown\n");
    
    switch ((int)this->platform->get(MOTOR_ERROR_UUID)){
        case 1:
            this->display->printf("MOTOR NOT RESPONDING");
            break;
        case 2:
            this->display->printf("CALIBRATION ERROR");
            //TODO: automated recalibration procedure?
            // this->platform->set(CENTRAL_DIMMER_UUID, 40.0);
            // delay(2000);
            // this->platform->set(CENTRAL_DIMMER_UUID, 0.0);
            // delay(1000);
            // this->platform->set(MOTOR_ERROR_UUID, 4.0);
            break;
        case 3:
            this->display->printf("MOTOR CONTROL ERROR");
            break;
        case 4:
            this->display->printf("RECALIBRATING");
            break;
        // default:
        //     if (this->platform->get(PRESSURE_SENSOR_ERROR_UUID) != 0.0)
        //         //TODO: replace with jam detected
        //         this->display->printf("PRESSURE ERROR: %i\n", (int)this->platform->get(PRESSURE_SENSOR_ERROR_UUID));
    }
    
    #if DEVELOPER_SCREEN
        this->display->setCursor(0, 16);
        float current_angle = this->platform->get(SERVO_ANGLE_UUID);
        
        this->display->printf("S: %i ", (int)current_angle);
        float valve_state = this->platform->get(VALVE_STATE_UUID);
        if (valve_state == 0.0)
            this->display->printf("V: HOLD \n");
        else if (valve_state == 1.0)
            this->display->printf("V: DRAIN \n");
        else if (valve_state == 2.0)
            this->display->printf("V: FILL \n");
        this->display->printf("Position: %.1f rev\n", this->platform->get(MOTOR_POSITION_UUID));
        
        this->display->printf("Vel: %.1f\n", this->platform->get(MOTOR_VELOCITY_UUID));
        this->display->setCursor(0, 40);
        this->display->printf("Torque: %.2f Nm\n", this->platform->get(MOTOR_TORQUE_UUID));

        #if PLATFORM_TYPE == 0
            this->display->printf("Voltage: %.1f V\n", this->platform->get(CENTRAL_DIMMER_UUID));
            float p1 = std::max((float)0.0, this->platform->get(PRESSURE_SENSOR_UUID));
            float p2 = std::max((float)0.0, this->platform->get(PRESSURE_SENSOR2_UUID));
            this->display->printf("PSI: %.2f | %.2f \n", p1, p2);
        #else
            this->display->printf("Voltage: %.1f, %.1f\n", this->platform->get(CENTRAL_DIMMER_UUID), this->platform->get(OUTER_DIMMER_UUID));
            if (this->platform->get(PRESSURE_SENSOR_ERROR_UUID) != 0.0)
                this->display->printf("PRESSURE ERROR");
            else
                this->display->printf("Pressure: %.2f PSI\n", std::max((float)0.0, this->platform->get(PRESSURE_SENSOR_UUID)));
        #endif
        
    #else
        if (this->current_menu == MenuType::BRADEN){
            this->display->clearDisplay();
            this->display->setCursor(0, 0);
            this->display->printf("Braden Score = %i\n", std::accumulate(std::begin(this->braden_score), std::end(this->braden_score), 0));
            for (int i = 0; i < 6; i++) {
                this->display->printf("  %s: \n", BRADEN_TITLES[i].c_str());
            }
            for (int i = 0; i < 6; i++){
                this->display->setCursor(80, 8*i+8);
                if (this->currently_selected != i || !this->editing || this->blink)
                    this->display->printf("%i", this->braden_score[i]);
            }
            this->display->setCursor(0, this->currently_selected*8 + 8);
            this->display->printf("->");
        } else {
            float progress;
            if (this->platform->get(AUTO_CONTROL_MODE_UUID) == 3.0)
                progress = constrain((128.0 - this->platform->get(SERVO_ANGLE_UUID)) / 42, 0, 1);
            else if (this->platform->get(AUTO_CONTROL_MODE_UUID) == 4.0)
                progress = 1.0;
            else if (this->platform->get(AUTO_CONTROL_MODE_UUID) == 5.0 || this->platform->get(AUTO_CONTROL_MODE_UUID) == 6.0)
                progress = 1.0-this->platform->get(AUTO_CONTROL_PROGRESS_UUID);
            else
                progress = this->platform->get(AUTO_CONTROL_PROGRESS_UUID);
            this->display->setCursor(0, 24);
            this->display->printf("Progress: %.1f%%\n", progress * 100.0);
            this->display->fillRect(0, 32 + 1, (int16_t)(DISPLAY_WIDTH * progress), 8 - 2, SSD1306_WHITE);
            this->display->drawRect(0, 32 + 1, DISPLAY_WIDTH, 8 - 2, SSD1306_WHITE);
            // struct tm timeinfo;
            // if (getLocalTime(&timeinfo, 0)) {   // 0ms timeout: never block the render loop
            //     char clock_buf[6];
            //     strftime(clock_buf, sizeof(clock_buf), "%H:%M", &timeinfo);
            //     this->display->setCursor(0, 40);
            //     this->display->printf("Time: %s", clock_buf);
            // }
            this->display->setCursor(0, 42);
            this->display->printf("Braden Score = %i\n", std::accumulate(std::begin(this->braden_score), std::end(this->braden_score), 0));
        }
    #endif
    
    #if BATTERY_MODE == 1
        this->display->drawBitmap(120, 0, battery_icon, 8, 13, WHITE);
        int bat = (int)(this->power->get_battery_percentage()*0.1);
        this->display->fillRect(122, 2+(10-bat), 5, bat, WHITE);
        // this->display->setCursor(106, 0);
        // this->display->printf("BAT");
        // this->display->setCursor(104, 8);
        // this->display->printf("%d%%", this->power->get_battery_percentage());
        
        if ((digitalRead(CHARGE_DETECT_PIN) == LOW)) { //if charging
            this->display->drawBitmap(119, 56, lightning_icon, 8, 8, WHITE); 
            //this->display->display();
        } 
        // else {
        //     this->display->fillRect(119, 56, 8, 8, BLACK); 
        //     this->display->display();
        // }
    #endif
    this->display->display();
    
}

void ControlPanel::update()
{
    if (millis()-this->prev_time >= BLINK_INTERVAL){
        this->blink = !this->blink;
        this->prev_time = millis();
    }
    this->update_knobs();
    this->update_buttons();
    #if PLATFORM_TYPE == 1 || BATTERY_MODE == 1
        this->update_joystick();
    #endif
    this->update_display();
}
