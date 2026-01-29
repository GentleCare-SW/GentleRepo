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
#include "tension_controller.h"
#include "config.h"
#include "service.h"



TensionController::TensionController()
{
    this->dimmer = nullptr;
    this->dimmer2 = nullptr;
    this->motor = nullptr;
    this->pressure_sensor = nullptr;
    this->voltage = 0.0;
    this->bumper_voltage = 0.0;
    this->torque_reference = 0.0;
}

TensionController::TensionController(const char *progress_uuid, VoltageDimmer *dimmer, VoltageDimmer *dimmer2, MotorController *motor, PressureSensor *pressure_sensor, float torque_reference)
{
    this->dimmer = dimmer;
    this->motor = motor;
    this->pressure_sensor = pressure_sensor;
    this->voltage = 0.0;
    this->bumper_voltage = 0.0;
    this->velocity = 0.0;
    this->min_velocity = 6.0;
    this->max_velocity = 30.0;
    this->v_kp = 25;
    this->bv_kp = .2;
    this->vel_kp = 0;
    this->torque_reference = torque_reference;
    this->add_characteristic(progress_uuid, nullptr, std::bind(&TensionController::get_progress, this));
}

void TensionController::update(float dt)
{
    float progress = this->get_progress();
    float torque_ref = constrain(2.25-(progress*2.0), 0, 1.25);
    float torque = this->motor->get_torque();
    float error = (torque_ref - torque);
    
    if (error < 0) {
        this->vel_kp = this->vel_kp/2;
        this->v_kp = this->vel_kp/3;
    } else{
        this->v_kp = 25;
        this->vel_kp = 0;
    }

    // float voltage_pid = -(torque - this->torque_reference) * this->v_kp;
    // float bumper_voltage_pid = -(torque - this->torque_reference) * this->bv_kp;
    // float velocity_pid = (torque - (this->torque_reference - 0.1)) * .8;
    if (progress >= .1)
        this->voltage = constrain(BASE_VOLTAGE + (error * this->v_kp), EVERSION_MIN_VOLTAGE, EVERSION_MAX_VOLTAGE);
    else this->voltage = 90;
            
    // this->bumper_voltage = constrain(this->bumper_voltage + bumper_voltage_pid, EVERSION_BUMPER_MIN_VOLTAGE, EVERSION_BUMPER_MAX_VOLTAGE);
    this->velocity = constrain(BASE_SPEED + (error * this->vel_kp), this->min_velocity, 30);
    this->dimmer->set_voltage(this->voltage);
    this->motor->set_velocity(this->velocity);
}

void TensionController::set_reference(float reference)
{
    this->torque_reference = reference;
}

float TensionController::get_reference()
{
    return this->torque_reference;
}

void TensionController::set_max_velocity(float max_velocity)
{
    this->max_velocity = max_velocity;
}

void TensionController::reset()
{
    this->voltage = 0.0;
    this->velocity = 0.0;
    this->dimmer->set_voltage(0.0);
    this->motor->set_velocity(0.0);
}

float TensionController::get_progress()
{
    //return max(0.0, constrain(this->motor->get_position() / SHEET_LENGTH, 0.0, 1.0));
    return constrain(pow((constrain(this->motor->get_position() / SHEET_LENGTH, 0.0, 1.0) ), 0.53), 0.0, 1.0);
}