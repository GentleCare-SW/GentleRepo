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

void pressure_controller_initialize(pressure_controller_t *controller, valve_t *valve, pressure_sensor_t *sensor)
{
    controller->valve = valve;
    controller->sensor = sensor;
    controller->valve_percentage = 0.0;
    controller->pressure_reference = 0.0;
}

void pressure_controller_set_reference(pressure_controller_t *controller, float reference)
{
    controller->pressure_reference = reference;
    if (reference == 0.0) {
        controller->valve_percentage = 0.0;
        valve_set_percentage(controller->valve, 0.0);
    }
}

float pressure_controller_get_reference(pressure_controller_t *controller)
{
    return controller->pressure_reference;
}

void pressure_controller_update(pressure_controller_t *controller)
{
    if (controller->pressure_reference > 0.0) {
        float pressure = pressure_sensor_get_pressure(controller->sensor);
        float pressure_derivative = pressure_sensor_get_derivative(controller->sensor);
        controller->valve_percentage += (controller->pressure_reference - pressure) * Kp - pressure_derivative * Kd;
        controller->valve_percentage = constrain(controller->valve_percentage, 0.0, 1.0);
        valve_set_percentage(controller->valve, controller->valve_percentage);
    }
}