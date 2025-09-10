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

#include "auto_controller.h"
#include "vessel.h"
#include "config.h"
#include "common/uuids.h"

AutoController::AutoController(const char *mode_uuid, const char *progress_uuid, VoltageDimmer *dimmer, MotorController *motor, PressureSensor *pressure_sensor, Servo *servo)
{
    this->dimmer = dimmer;
    this->motor = motor;
    this->pressure_sensor = pressure_sensor;
    this->servo = servo;
    this->tension_controller = TensionController(dimmer, motor, pressure_sensor, REFERENCE_TORQUE);
    this->set_mode((float)AutoControlMode::IDLE);

    this->add_characteristic(mode_uuid, std::bind(&AutoController::set_mode, this, std::placeholders::_1), std::bind(&AutoController::get_mode, this));
    this->add_characteristic(progress_uuid, nullptr, std::bind(&AutoController::get_progress, this));
}

void AutoController::update(float dt)
{
    float progress = this->get_progress();
    float max_speed = constrain(progress / 0.15, 0.0, 1.0) * 16.0 + 4.0;

    if (this->mode == AutoControlMode::EVERSION) {
        this->tension_controller.update(dt);

        if (progress >= 1.0)
            this->set_mode((float)AutoControlMode::IDLE);
        else
            this->tension_controller.set_max_velocity(max_speed);
    } else if (this->mode == AutoControlMode::INVERSION) {
        if (progress <= 0.0)
            this->set_mode((float)AutoControlMode::IDLE);
        else
            this->motor->set_velocity(-max_speed);
    }
}

void AutoController::mode_changed(VesselMode mode)
{
    this->set_mode((float)AutoControlMode::IDLE);
}

void AutoController::set_mode(float mode)
{
    int chamber = this->servo->get_chamber();

    this->mode = (AutoControlMode)mode;
    if (this->mode == AutoControlMode::IDLE) {
        this->dimmer->set_percentage(0.0);
    } else if (this->mode == AutoControlMode::EVERSION) {
        this->dimmer->set_percentage(chamber == 0 ? 0.0 : 0.3);
    } else if (this->mode == AutoControlMode::EVERSION_PAUSED) {
        this->dimmer->set_percentage(EVERSION_PAUSED_VOLTAGE);
    } else if (this->mode == AutoControlMode::INVERSION) {
        this->dimmer->set_percentage(chamber == 0 ? INVERSION_VOLTAGE : 0.0);
    } else if (this->mode == AutoControlMode::INVERSION_PAUSED) {
        this->dimmer->set_percentage(chamber == 0 ? INVERSION_PAUSED_VOLTAGE : 0.0);
    }

    this->motor->set_velocity(0.0);
}

float AutoController::get_mode()
{
    return (float)this->mode;
}

float AutoController::get_progress()
{
    return constrain(this->motor->get_position() / SHEET_LENGTH, 0.0, 1.0);
}