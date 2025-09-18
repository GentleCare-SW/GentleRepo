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

#pragma once
#include "common/uuids.h"
#include <NimBLEDevice.h>

class RemoteVessel: public NimBLEScanCallbacks {
public:
    RemoteVessel();

    void start();

    void update();

    void onResult(const NimBLEAdvertisedDevice *device) override;

    void on_notification(NimBLERemoteCharacteristic *characteristic, uint8_t *data, size_t length, bool isNotify);

    float get(const char *uuid);

    void set(const char *uuid, float velocity);

private:
    NimBLERemoteCharacteristic *get_characteristic(const char *uuid);

    float values[CHARACTERISTIC_UUID_COUNT];

    NimBLEScan *scanner;
    NimBLEClient *client;
    bool found_device;
    NimBLEAdvertisedDevice *device;
    NimBLERemoteService *service;
    NimBLERemoteCharacteristic *characteristics[CHARACTERISTIC_UUID_COUNT];
};