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

#include "pressure_controller.h"
#include "config.h"

static const float Kp = 100.0;
static const float Ki = 5.0;
static const float Kd = 10.0;

PressureController::PressureController(VoltageDimmer *dimmer, PressureSensor *sensor)
{
    this->dimmer = dimmer;
    this->sensor = sensor;
    this->voltage = 0.0;
    this->pressure_reference = 0.0;
    this->controller_error = false;
}

PressureController::PressureController(const char *uuid, VoltageDimmer *dimmer, PressureSensor *sensor)
{
    *this = PressureController(dimmer, sensor);
    this->add_characteristic(uuid, std::bind(&PressureController::set_reference, this, std::placeholders::_1), std::bind(&PressureController::get_reference, this));
}

void PressureController::update(float dt)
{
    Peripheral::update(dt);

    if (this->pressure_reference == 0.0 || this->sensor->get_error() == 1.0)
        return;

    float pressure = this->sensor->get_pressure();
    float pressure_derivative = this->sensor->get_derivative();

    error = pressure_reference - pressure;  //error should never be more than 1
    d_error = (error - prev_error) / dt;
    prev_error = error;   
    i_error += constrain(error * dt, -2.5, 2.5); 

    float Pcontrol = error * Kp;   
    float Icontrol = i_error * Ki;  
    float Dcontrol = d_error * Kd;
    
    this->voltage = constrain(Pcontrol + Icontrol + Dcontrol, 0.001, MAX_DIMMER_VOLTAGE);

    if (i_error == 2.5 && pressure == 0.0) {
        this->sensor->set_error(PressureSensorError::NOT_CONNECTED);
        this->controller_error = true;
        return;
    }
    
    this->dimmer->set_voltage(this->voltage);
}

void PressureController::mode_changed(ServiceMode mode)
{
    this->set_reference(0.0);
}

void PressureController::set_reference(float reference)
{
    this->pressure_reference = reference;
    if (reference == 0.0) {
        this->voltage = 0.0;
        this->dimmer->set_voltage(this->voltage);
    }
}

float PressureController::get_reference()
{
    return this->pressure_reference;
}

boolean PressureController::is_working()
{
    return this->controller_error == false;
}