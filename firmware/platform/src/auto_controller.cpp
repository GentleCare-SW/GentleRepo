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
#include "auto_controller.h"
#include "service.h"
#include "config.h"

AutoController::AutoController(const char *mode_uuid, const char *progress_uuid, VoltageDimmer *dimmer, VoltageDimmer *dimmer2, MotorController *motor, PressureSensor *pressure_sensor, Servo *servo)
{
    this->dimmer = dimmer;
    this->dimmer2 = dimmer2;
    this->motor = motor;
    this->pressure_sensor = pressure_sensor;
    this->servo = servo;
    this->tension_controller = TensionController(progress_uuid, dimmer, dimmer2, motor, pressure_sensor, REFERENCE_TORQUE);
    this->set_mode((float)AutoControlMode::IDLE);

    this->add_characteristic(mode_uuid, std::bind(&AutoController::set_mode, this, std::placeholders::_1), std::bind(&AutoController::get_mode, this));
    this->add_characteristic(progress_uuid, nullptr, std::bind(&AutoController::get_progress, this));
}

void AutoController::update(float dt)
{
    Peripheral::update(dt);
    
    float progress = this->get_progress();
    float max_speed = constrain(progress / 0.3, 0.0, 1.0) * 20.0 + 5.0;

    if (this->mode == AutoControlMode::EVERSION) {
        this->tension_controller.update(dt);
        if (progress >= 0.2)
            this->dimmer2->set_voltage(BUMPER_EVERSION_VOLTAGE);

        if (progress >= 1.0)
            this->set_mode((float)AutoControlMode::EVERSION_PAUSED);
        else
            this->tension_controller.set_max_velocity(max_speed);
    } else if (this->mode == AutoControlMode::INVERSION) {
        if (progress <= 0.0){
            this->set_mode((float)AutoControlMode::IDLE);
        } else if (progress <= 0.3){
            this->dimmer->set_voltage(1.2*progress);
            this->dimmer2->set_voltage(0);
            this->motor->set_velocity(-max_speed);
        } else{
            this->dimmer->set_voltage(INVERSION_VOLTAGE);
            this->dimmer2->set_voltage(BUMPER_INVERSION_VOLTAGE);
            this->motor->set_velocity(-max_speed);}
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
        this->dimmer->set_voltage(PAUSED_VOLTAGE);
        this->dimmer2->set_voltage(BUMPER_PAUSED_VOLTAGE);

    } else if (this->mode == AutoControlMode::INVERSION) {
        this->dimmer->set_voltage(INVERSION_VOLTAGE);
        this->dimmer2->set_voltage(BUMPER_INVERSION_VOLTAGE);

    } else if (this->mode == AutoControlMode::INVERSION_PAUSED) {
        this->dimmer->set_voltage(PAUSED_VOLTAGE);
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
    return constrain(pow((constrain(this->motor->get_position() / SHEET_LENGTH, 0.0, 1.0) ), 0.53), 0.0, 1.0);
}

void AutoController::toggle_paused()
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