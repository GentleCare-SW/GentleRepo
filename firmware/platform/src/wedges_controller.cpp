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

#include <math.h>
#include "wedges_controller.h"
#include "service.h"
#include "config.h"

static const float DEFAULT_HOLD_TIME = 1.0 * 60.0 * 1000.0;

WedgesController::WedgesController(const char *mode_uuid, const char *progress_uuid, const char *timer_uuid, 
    VoltageDimmer *dimmer, MotorController *motor, PressureSensor *pressure_sensor1, PressureSensor *pressure_sensor2, Servo *servo, Valve *valve, Steering *rail)
{
    this->dimmer = dimmer;
    this->motor = motor;
    this->valve = valve;
    this->pressure_sensor1 = pressure_sensor1;
    this->pressure_sensor2 = pressure_sensor2;
    this->servo = servo;
    this->rail = rail;
    this->rail_pin = RAIL_STATE_PIN;
    this->to_rail_pin = BOX_TO_RAIL_PIN;
    pinMode(this->rail_pin, INPUT);
    pinMode(this->to_rail_pin, OUTPUT);
    this->set_mode((float)AutoControlMode::IDLE);
    this->timer_active = false;

    this->add_characteristic(mode_uuid, std::bind(&WedgesController::set_mode, this, std::placeholders::_1), std::bind(&WedgesController::get_mode, this));
    this->add_characteristic(progress_uuid, nullptr, std::bind(&WedgesController::get_progress, this));
    this->add_characteristic(timer_uuid, nullptr, std::bind(&WedgesController::get_time, this)); 
}

void WedgesController::update(float dt)
{
    Peripheral::update(dt);
    
    float progress = this->get_progress();
    float max_speed = constrain(progress / 0.3, 0.0, 1.0) * 10.0 + 5.0;
    if (this->mode == AutoControlMode::IDLE && progress <= 0.02) {
        digitalWrite(this->to_rail_pin, LOW);
    } else {
        digitalWrite(this->to_rail_pin, HIGH);
    }

    if (this->mode == AutoControlMode::EVERSION) {
        if (digitalRead(this->rail_pin)==HIGH){
            this->auto_eversion(progress);
        } else if (this->rail->get_direction() != 1.0) {
            this->rail->set_direction(1.0);
        }

    } else if (this->mode == AutoControlMode::INVERSION) {
        this->auto_inversion(progress);
        
    } else if (this->mode == AutoControlMode::TRANSFER){
        this->valve->set_state((float)ValveState::FILL);
        this->dimmer->set_voltage(60.0);
        float pressure_sum = pressure_sensor1->get_pressure() + pressure_sensor2->get_pressure();
        float pressure_error = 0.9 - pressure_sum;
        this->servo->set_angle(this->servo->get_angle() + 10.0 * pressure_error * dt);

        if (pressure_sensor2->get_pressure() >= 1.08){
            this->set_mode((float)AutoControlMode::TRANSFER_PAUSED);
            this->timer_start = millis();
            this->timer_active = true;
            this->dimmer->set_voltage(0.0);
        }
    } else if (this->mode == AutoControlMode::TRANSFER_PAUSED){
        //pausing during filling ends up starting inversion
        if (timer_active && millis() - this->timer_start > DEFAULT_HOLD_TIME) {
            this->set_mode((float)AutoControlMode::INVERSION); 
            this->timer_active = false;
        }
    }
}

void WedgesController::mode_changed(ServiceMode mode)
{
    this->set_mode((float)AutoControlMode::IDLE);
}

void WedgesController::set_mode(float mode)
{
    if ((AutoControlMode)mode == this->mode && mode != (float)AutoControlMode::IDLE)
        return;

    this->mode = (AutoControlMode)mode;
    if (this->mode == AutoControlMode::IDLE) {
        this->dimmer->set_voltage(0);
        this->valve->set_state((float)ValveState::HOLD);

    } else if (this->mode == AutoControlMode::EVERSION) {
        //this->dimmer->set_voltage(BASE_VOLTAGE);
        this->valve->set_state((float)ValveState::HOLD);
        this->rail->set_direction(1.0);
        delay(200);

    } else if (this->mode == AutoControlMode::EVERSION_PAUSED) {
        this->dimmer->set_voltage(EVERSION_PAUSED_VOLTAGE);
        this->valve->set_state((float)ValveState::HOLD);

    } else if (this->mode == AutoControlMode::TRANSFER) {
        this->valve->set_state((float)ValveState::FILL);
    
    } else if (this->mode == AutoControlMode::TRANSFER_PAUSED) {
        this->valve->set_state((float)ValveState::HOLD);

    } else if (this->mode == AutoControlMode::INVERSION) {
        this->servo->set_angle(128.0);
        this->valve->set_state((float)ValveState::DRAIN);
        this->dimmer->set_voltage(INVERSION_VOLTAGE);

    } else if (this->mode == AutoControlMode::INVERSION_PAUSED) {
        this->dimmer->set_voltage(INVERSION_PAUSED_VOLTAGE);
        this->valve->set_state((float)ValveState::DRAIN);
    }
    this->motor->set_velocity(0.0);
}

float WedgesController::get_mode()
{
    return (float)this->mode;
}

void WedgesController::auto_eversion(float progress)
{
    if (progress >= 0.75)
        this->valve->set_state((float)ValveState::FILL); 

    if (progress >= 1.0){
        this->set_mode((float)AutoControlMode::TRANSFER); 
    } else {
        float torque_ref = constrain(-0.5*progress, -1.0, REFERENCE_TORQUE);
        float torque = this->motor->get_torque();
        float error = (torque_ref - torque);
        float v_kp = (error > 0) ? 50.0 : 25.0;
        float vel_kp = 0.0;
        float eversion_voltage;
        float eversion_velocity;

        if (progress >= 0.18){ 
            eversion_voltage = constrain(BASE_VOLTAGE + (error * v_kp), EVERSION_MIN_VOLTAGE, EVERSION_MAX_VOLTAGE);
            eversion_velocity = constrain(BASE_SPEED + (error * vel_kp), 5.0, 20.0);
        }else {
            eversion_voltage = 50;
            eversion_velocity = 10;
        }

        if (eversion_voltage >= 80) 
            eversion_velocity = BASE_SPEED + ((80 - eversion_voltage)* .1);
        
        if (torque < -1.0)
            eversion_velocity = 6.0;
        
        this->dimmer->set_voltage(eversion_voltage);
        this->motor->set_velocity(eversion_velocity);   
    }
}

void WedgesController::auto_inversion(float progress)
{
    float inversion_voltage = (1.0 - progress) * 32.0 + 20.0;
    if (progress <= 0.0){
        this->set_mode((float)AutoControlMode::IDLE);
        digitalWrite(this->to_rail_pin, LOW);
        delay(50);
        this->rail->set_direction(-1.0);
        delay(150);
    } else if (progress <= 0.02){
        this->dimmer->set_voltage(0.0);
        this->motor->set_velocity(-0.2);
    } else {
        float pressure = this->pressure_sensor2->get_pressure();
        float pressure_derivative = this->pressure_sensor2->get_derivative();
        float pressure_goal = (2.0 - 0.8*progress) * 0.2;
        float pressure_error = pressure_goal - pressure;
        // base_velocity + pressure_error * Kp - pressure_derivative * Kd;
        float inversion_velocity = constrain(-4.0 - pressure_error*20.0 + pressure_derivative*2.5, -10.0, -0.5);

        this->dimmer->set_voltage(inversion_voltage);
        this->motor->set_velocity(inversion_velocity);
    }
}

float WedgesController::get_progress()
{
    return constrain(pow((constrain(this->motor->get_position() / SHEET_LENGTH, 0.0, 1.0) ), 0.676), 0.0, 1.0);
}

long WedgesController::get_time()
{
    if (this->mode != AutoControlMode::TRANSFER_PAUSED)
        return 0;
    else
        return DEFAULT_HOLD_TIME - (millis() - this->timer_start);
}

void WedgesController::toggle_paused()  //TODO: is this being used?
{
    if (this->mode == AutoControlMode::EVERSION)
        this->set_mode((float)AutoControlMode::EVERSION_PAUSED);
    else if (this->mode == AutoControlMode::EVERSION_PAUSED)
        this->set_mode((float)AutoControlMode::EVERSION);
    else if (this->mode == AutoControlMode::INVERSION)
        this->set_mode((float)AutoControlMode::INVERSION_PAUSED);
    else if (this->mode == AutoControlMode::INVERSION_PAUSED)
        this->set_mode((float)AutoControlMode::INVERSION);
}