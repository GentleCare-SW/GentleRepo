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
#include "pressure_sensor.h"

static const int32_t ADC_MAX_VALUE = 4095;
static const float ADC_VOLTAGE_REF = 5.0;
static const float MIN_VOLTAGE = 0.5; 
static const float MAX_VOLTAGE = 4.5;
static const float MIN_PSI = 0.0;
static const float MAX_PSI = 100.0; 
static const float EMA_ALPHA = 0.001;

void pressure_sensor_initialize(pressure_sensor_t *sensor, int32_t adc_pin, float pressure_constant)
{
    sensor->adc_pin = adc_pin;
    sensor->moving_pressure = 0.0;
    sensor->previous_read_time = micros();
    sensor->pressure_derivative = 0.0;
    sensor->pressure_offset = 0.0;
    sensor->pressure_constant = pressure_constant;
    pinMode(sensor->adc_pin, INPUT);
}

void pressure_sensor_update(pressure_sensor_t *sensor, bool calibrate)
{
    int32_t adc_value = analogRead(sensor->adc_pin);
    float voltage = adc_value * ADC_VOLTAGE_REF / ADC_MAX_VALUE;
    float psi = (voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * (MAX_PSI - MIN_PSI) + MIN_PSI;

    float previous_moving_pressure = sensor->moving_pressure;
    sensor->moving_pressure = sensor->moving_pressure * (1.0 - EMA_ALPHA) + psi * EMA_ALPHA;

    int64_t current_time = micros();
    float dt = (float)(current_time - sensor->previous_read_time) / 1e6;
    sensor->previous_read_time = current_time;

    sensor->pressure_derivative = (sensor->moving_pressure - previous_moving_pressure) / dt;

    if (calibrate)
        sensor->pressure_offset = sensor->pressure_offset * (1.0 - EMA_ALPHA) + sensor->moving_pressure * EMA_ALPHA;
}

float pressure_sensor_get_pressure(pressure_sensor_t *sensor)
{
    return (sensor->moving_pressure - sensor->pressure_offset) * sensor->pressure_constant;
}

float pressure_sensor_get_derivative(pressure_sensor_t *sensor)
{
    return sensor->pressure_derivative * sensor->pressure_constant;
}