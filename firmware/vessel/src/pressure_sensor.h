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

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

typedef struct pressure_sensor {
    int32_t adc_pin;
    float moving_pressure;
    int64_t previous_read_time;
    float pressure_derivative;
    float pressure_offset;
    float pressure_constant;
} pressure_sensor_t;

void pressure_sensor_initialize(pressure_sensor_t *sensor, int32_t adc_pin, float pressure_constant);

void pressure_sensor_update(pressure_sensor_t *sensor, bool calibrate = false);

float pressure_sensor_get_pressure(pressure_sensor_t *sensor);

float pressure_sensor_get_derivative(pressure_sensor_t *sensor);

#endif