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

static const int32_t WINDOW_SIZE = 1024;

static int32_t adc_pin = -1;
static float moving_pressure = 0.0;
static float previous_moving_pressure = 0.0;
static float EMA_ALPHA = 0.001;

void pressure_sensor_initialize(int32_t adc)
{
    adc_pin = adc;
    pinMode(adc_pin, INPUT);
}
 
float pressure_sensor_read()
{
    int32_t adc_value = analogRead(adc_pin);
    float voltage = adc_value * ADC_VOLTAGE_REF / ADC_MAX_VALUE;
    float psi = (voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * (MAX_PSI - MIN_PSI) + MIN_PSI;

    previous_moving_pressure = moving_pressure;
    moving_pressure = moving_pressure * (1.0 - EMA_ALPHA) + psi * EMA_ALPHA;

    return moving_pressure;
}

float pressure_sensor_read_derivative()
{
    return moving_pressure - previous_moving_pressure; 
}