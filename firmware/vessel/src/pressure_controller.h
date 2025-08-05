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

#ifndef PRESSURE_CONTROLLER_H
#define PRESSURE_CONTROLLER_H

#include "valve.h"
#include "pressure_sensor.h"

typedef struct pressure_controller {
    float valve_percentage;
    float pressure_reference;
    pressure_sensor_t *sensor;
    valve_t *valve;
} pressure_controller_t;

void pressure_controller_initialize(pressure_controller_t *controller, valve_t *valve, pressure_sensor_t *sensor);

void pressure_controller_set_reference(pressure_controller_t *controller, float reference);

float pressure_controller_get_reference(pressure_controller_t *controller);

void pressure_controller_update(pressure_controller_t *controller);

#endif