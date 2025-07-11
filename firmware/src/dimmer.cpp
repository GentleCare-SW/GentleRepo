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
#include "dimmer.h"

static const int32_t HALF_CYCLE_TIME = 8333;
static const float MAX_POWER = 0.5;

static int32_t zc_pin = -1;
static int32_t psm_pin = -1;
static int64_t zero_cross_time = -1;
static int32_t delay_time = HALF_CYCLE_TIME;

static void zero_cross_isr()
{
    zero_cross_time = micros();
}

void dimmer_initialize(int32_t zc, int32_t psm)
{
    zc_pin = zc;
    psm_pin = psm;

    pinMode(psm_pin, OUTPUT);
    pinMode(zc_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(zc_pin), zero_cross_isr, RISING);
}

void dimmer_set_power(float power)
{
    if (power > MAX_POWER)
        power = MAX_POWER;

    if (power < 0.0)
        power = 0.0;
    
    delay_time = (int32_t)(HALF_CYCLE_TIME * (1.0 - power));
}

void dimmer_update()
{
    if (zero_cross_time < 0 || delay_time == HALF_CYCLE_TIME)
        return;
    
    int64_t current_time = micros();
    if (current_time >= zero_cross_time + delay_time) {
        digitalWrite(psm_pin, HIGH);
        delayMicroseconds(10);
        digitalWrite(psm_pin, LOW);
        zero_cross_time = -1;
    } 
}