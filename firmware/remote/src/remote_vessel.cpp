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
#include "remote_vessel.h"
#include "common/uuids.h"

RemoteVessel::RemoteVessel()
{
    found_device = false;
}

void RemoteVessel::start()
{
    NimBLEDevice::init("GentleBombDetonator");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    this->scanner = NimBLEDevice::getScan();
    this->scanner->setScanCallbacks(this);
    this->scanner->setActiveScan(true);
    this->scanner->setMaxResults(0);

    this->client = NimBLEDevice::createClient();
}

void RemoteVessel::update()
{
    while (!this->client->isConnected()) {
        if (!this->scanner->isScanning())
            this->scanner->start(0);

        if (this->found_device) {
            this->found_device = false;
            this->scanner->stop();

            this->client->connect(this->device);
            this->client->setConnectionParams(6, 12, 0, 100);
            this->service = this->client->getService(VESSEL_UUID);
            for (int i = 0; i < CHARACTERISTIC_UUID_COUNT; i++) {
                this->characteristics[i] = this->service->getCharacteristic(CHARACTERISTIC_UUIDS[i]);
                if (this->characteristics[i] != nullptr && this->characteristics[i]->canNotify())
                    this->characteristics[i]->subscribe(true, std::bind(&RemoteVessel::on_notification, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
                this->values[i] = 0.0;
            }
        }
    }
}

void RemoteVessel::onResult(const NimBLEAdvertisedDevice *device)
{
#if DEBUG_MODE
    Serial.printf("Device: %s\n", device->toString().c_str());
#endif
    if (device->getServiceUUID().toString() != VESSEL_UUID)
        return;
    
    this->device = (NimBLEAdvertisedDevice *)device;
    this->found_device = true;
}

void RemoteVessel::on_notification(NimBLERemoteCharacteristic *characteristic, uint8_t *data, size_t length, bool isNotify)
{
    if (length != sizeof(float))
        return;

    for (int i = 0; i < CHARACTERISTIC_UUID_COUNT; i++) {
        if (strcmp(CHARACTERISTIC_UUIDS[i], characteristic->getUUID().toString().c_str()) == 0)
            this->values[i] = *(float *)data;
    }
}

NimBLERemoteCharacteristic *RemoteVessel::get_characteristic(const char *uuid)
{
    for (int i = 0; i < CHARACTERISTIC_UUID_COUNT; i++) {
        if (strcmp(CHARACTERISTIC_UUIDS[i], uuid) == 0)
            return this->characteristics[i];
    }

    return nullptr;
}

float RemoteVessel::get(const char *uuid)
{
    for (int i = 0; i < CHARACTERISTIC_UUID_COUNT; i++) {
        if (strcmp(CHARACTERISTIC_UUIDS[i], uuid) == 0)
            return this->values[i];
    }

    return 0.0;
}

void RemoteVessel::set(const char *uuid, float value)
{
    if (!this->client->isConnected())
        return;
    
    NimBLERemoteCharacteristic *characteristic = this->get_characteristic(uuid);
    if (characteristic == nullptr)
        return;

    characteristic->writeValue((uint8_t *)&value, sizeof(value));
}