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

#include "peripheral.h"
#include "vessel.h"

void Peripheral::start()
{
}

void Peripheral::add_characteristic(const char *uuid, std::function<void(float)> setter, std::function<float()> getter)
{
    if (this->characteristic_count < MAX_CHARACTERISTICS)
        this->characteristics[this->characteristic_count++] = Characteristic(uuid, setter, getter);
}

void Peripheral::update(float dt)
{
    for (int i = 0; i < this->characteristic_count; i++)
        this->characteristics[i].notify();
}

void Peripheral::mode_changed(VesselMode mode)
{
}