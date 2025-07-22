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
#include "dimmer.h"

static float dimmer_power = 0.0;
static float pressure_reference = 0.0;

static const float Kp = 0.000025;
static const float Kd = 0.00003;

void pressure_controller_set_reference(float reference)
{
    pressure_reference = reference;
    if (reference == 0.0) {
        dimmer_power = 0.0;
        dimmer_set_power(0.0);
    }
}

float pressure_controller_get_reference()
{
    return pressure_reference;
}

void pressure_controller_update(float pressure, float pressure_derivative)
{
    if (pressure_reference > 0.0) {
        dimmer_power += (pressure_reference - pressure) * Kp - pressure_derivative * Kd;
        dimmer_power = constrain(dimmer_power, 0.0, 0.5);
        dimmer_set_power(dimmer_power);
    }
}