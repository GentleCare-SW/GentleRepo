/*
 * Copyright (c) 2026 GentleCare Corporation. All rights reserved.
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
#include "auto_controller.h"
#include "service.h"
#include "config.h"

AutoController::AutoController(const char *mode_uuid, const char *progress_uuid, VoltageDimmer *dimmer, VoltageDimmer *dimmer2, MotorController *motor, PressureSensor *pressure_sensor, PressureController *pressure_controller)
{
    this->dimmer = dimmer;
    this->dimmer2 = dimmer2;
    this->motor = motor;
    this->pressure_sensor = pressure_sensor;
    this->pressure_controller = pressure_controller;
    this->tension_controller = TensionController(progress_uuid, dimmer, dimmer2, motor, pressure_sensor);
    this->set_mode((float)AutoControlMode::IDLE);

    this->add_characteristic(mode_uuid, std::bind(&AutoController::set_mode, this, std::placeholders::_1), std::bind(&AutoController::get_mode, this));
    this->add_characteristic(progress_uuid, nullptr, std::bind(&AutoController::get_progress, this));
}

void AutoController::update(float dt)
{
    Peripheral::update(dt);
    
    float progress = this->get_progress();
    float max_speed = constrain(progress / 0.4, 0.0, 1.0) * 20.0 + 5.0;
    //float inversion_speed = constrain(progress / 0.8, 0.0, 1.0) * 7.0 + 5.0;

    if (this->mode == AutoControlMode::EVERSION) {
        this->tension_controller.update(dt);
        
        if (progress >= 1.0 || progress >= .87 && this->dimmer->get_voltage() > 115)
            this->set_mode((float)AutoControlMode::TRANSFER_PAUSED);
        else
            this->tension_controller.set_max_velocity(max_speed);
    } else if (this->mode == AutoControlMode::INVERSION) {
        if (this->motor->get_position() <= 0.0){
            this->set_mode((float)AutoControlMode::IDLE);
        } else{
            #if SOFTWARE_VERSION == 1
                float pressure_setpoint;
                float inversion_velocity = progress > 0.85 ? -8.5 : -10.0;
                // if (progress > .85)
                //     pressure_setpoint = .4;
                // else if (progress > .6)
                //     pressure_setpoint = .5;
                // else if (progress > .4)
                //     pressure_setpoint = .7;
                // else {
                //     pressure_setpoint = constrain(2*progress-.4, 0, 1);
                //     this->dimmer2->set_voltage(0.0);
                // }
                if (progress > .9)
                    pressure_setpoint = .4;
                else if (progress > .55)
                    pressure_setpoint = (-0.85*progress) + 1.12;
                else {
                    pressure_setpoint = constrain(2*progress-.4, 0, 1);
                    this->dimmer2->set_voltage(0.0);
                }
                if (this->motor->get_torque() > 20.0)
                    inversion_velocity = -7.5;


                pressure_controller->set_reference(pressure_setpoint);
                this->motor->set_velocity(inversion_velocity);
            #else
            float inversion_voltage = constrain(112.0*progress, 0, 75);
            float tension_ref = constrain(30*(1-progress), 10, 28);
            float tension_error = tension_ref - this->motor->get_torque();
            float inversion_velocity = constrain(-5.0 - tension_error*2.0, -20.0, -4.0);

            this->dimmer->set_voltage(inversion_voltage);
            this->dimmer2->set_voltage((progress <= 0.3) ? 0.0 : BUMPER_INVERSION_VOLTAGE);
            this->motor->set_velocity(inversion_velocity);
            #endif
        }
    }
}

void AutoController::mode_changed(ServiceMode mode)
{
    this->set_mode((float)AutoControlMode::IDLE);
}

void AutoController::set_mode(float mode)
{
    if ((AutoControlMode)mode == this->mode && mode != (float)AutoControlMode::IDLE)
        return;

    this->mode = (AutoControlMode)mode;
    if (this->mode == AutoControlMode::IDLE) {
        this->dimmer->set_voltage(0);
        this->dimmer2->set_voltage(0);

    } else if (this->mode == AutoControlMode::EVERSION) {
        this->tension_controller.reset();
        this->dimmer->set_voltage(BASE_VOLTAGE);
        this->dimmer2->set_voltage(0.0);

    } else if (this->mode == AutoControlMode::EVERSION_PAUSED) {
        this->dimmer->set_voltage(EVERSION_PAUSED_VOLTAGE);
        this->dimmer2->set_voltage(BUMPER_PAUSED_VOLTAGE);

    } else if (this->mode == AutoControlMode::TRANSFER) {
        this->dimmer->set_voltage(30.0);
        this->dimmer2->set_voltage(INVERSION_PAUSED_VOLTAGE);

    } else if (this->mode == AutoControlMode::TRANSFER_PAUSED) {
        this->dimmer->set_voltage(EVERSION_PAUSED_VOLTAGE);
        this->dimmer2->set_voltage(BUMPER_PAUSED_VOLTAGE);

    } else if (this->mode == AutoControlMode::INVERSION) {
        this->dimmer->set_voltage(INVERSION_VOLTAGE);
        this->dimmer2->set_voltage(BUMPER_INVERSION_VOLTAGE);

    } else if (this->mode == AutoControlMode::INVERSION_PAUSED) {
        this->dimmer->set_voltage(INVERSION_PAUSED_VOLTAGE);
        this->dimmer2->set_voltage(BUMPER_PAUSED_VOLTAGE);
    }

    this->motor->set_velocity(0.0);
}

float AutoController::get_mode()
{
    return (float)this->mode;
}

float AutoController::get_progress()
{
    return constrain(pow((constrain(this->motor->get_position() / SHEET_LENGTH, 0.0, 1.0) ), PROGRESS_FACTOR), 0.0, 1.0);
}
