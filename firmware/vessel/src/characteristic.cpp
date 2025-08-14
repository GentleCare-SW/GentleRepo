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

#include "characteristic.h"

Characteristic::Characteristic()
{
    this->characteristic = nullptr;
    this->setter = nullptr;
    this->getter = nullptr;
    this->uuid = nullptr;
}

Characteristic::Characteristic(const char *uuid, std::function<void(float)> setter, std::function<float()> getter)
{
    this->uuid = uuid;
    this->setter = setter;
    this->getter = getter;
    this->characteristic = nullptr;
}

void Characteristic::onRead(NimBLECharacteristic *characteristic, NimBLEConnInfo& info)
{
    if (this->getter == nullptr)
        return;

    float value = this->getter();
    characteristic->setValue(value);
}

void Characteristic::onWrite(NimBLECharacteristic *characteristic, NimBLEConnInfo& info)
{
    NimBLEAttValue characteristic_value = characteristic->getValue();
    if (this->setter == nullptr || characteristic_value.length() != sizeof(float))
        return;

    float value;
    memcpy(&value, characteristic_value.data(), sizeof(float));
    this->setter(value);
}