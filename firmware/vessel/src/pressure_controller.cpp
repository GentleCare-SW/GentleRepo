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
#include "pressure_controller.h"

static const float Kp = 0.000025;
static const float Kd = 0.00003;

PressureController::PressureController(VoltageDimmer *dimmer, PressureSensor *sensor)
{
    this->dimmer = dimmer;
    this->sensor = sensor;
    this->percentage = 0.0;
    this->pressure_reference = 0.0;
}

PressureController::PressureController(const char *uuid, VoltageDimmer *dimmer, PressureSensor *sensor)
{
    *this = PressureController(dimmer, sensor);
    this->add_characteristic(uuid, std::bind(&PressureController::set_reference, this, std::placeholders::_1), std::bind(&PressureController::get_reference, this));
}

void PressureController::update(float dt)
{
    if (this->pressure_reference == 0.0)
        return;

    float pressure = this->sensor->get_pressure();
    float pressure_derivative = this->sensor->get_derivative();
    this->percentage += (this->pressure_reference - pressure) * Kp - pressure_derivative * Kd;
    this->percentage = constrain(this->percentage, 0.0, 1.0);
    this->dimmer->set_percentage(this->percentage);
}

void PressureController::mode_changed(VesselMode mode)
{
    this->set_reference(0.0);
}

void PressureController::set_reference(float reference)
{
    this->pressure_reference = reference;
    if (reference == 0.0) {
        this->percentage = 0.0;
        this->dimmer->set_percentage(this->percentage);
    }
}

float PressureController::get_reference()
{
    return this->pressure_reference;
}