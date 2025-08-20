/*
 * Copyright (c) 2025 GentleCare Corporation. All rights reserved.
 *
 * This source code and the accompanying materials are the confidential and
 * proprietary information of GentleCare Corporation. Unauthorized copying or
 * distribution of this file, via any medium, is strictly prohibited without
 * the prior written permission of John Payne.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

import CoreBluetooth

enum VesselMode: Float {
    case idle = 0.0
    case eversion = 1.0
    case eversionPaused = 2.0
    case inversion = 3.0
    case inversionPaused = 4.0
}

class RemoteVessel: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    @Published var bluetoothStatus: String = "Starting..."
    @Published var isConnected: Bool = false
    @Published var mode: VesselMode?
    @Published var chamber: Float?
    @Published var progress: Float?
    @Published var airPressure: Float?
    @Published var motorTorque: Float?
    
    private var central: CBCentralManager!
    private var peripheral: CBPeripheral?
    
    override init() {
        super.init()
        central = CBCentralManager(delegate: self, queue: nil)
    }
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            bluetoothStatus = "Bluetooth ON. Scanning..."
            startScan()
        case .poweredOff:
            bluetoothStatus = "Bluetooth is OFF. Please enable it in Settings."
        case .unauthorized:
            bluetoothStatus = "No Bluetooth permission. Grant it in Settings."
        case .unsupported:
            bluetoothStatus = "This device doesn't support Bluetooth."
        case .resetting:
            bluetoothStatus = "Bluetooth resetting..."
        default:
            bluetoothStatus = "Bluetooth state unknown."
        }
    }
    
    private func startScan() {
        if central.state != .poweredOn {
            return;
        }
        
        central.scanForPeripherals(withServices: [VESSEL_UUID], options: [CBCentralManagerScanOptionAllowDuplicatesKey: false])
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        bluetoothStatus = "Discovered device. Connecting..."
        self.peripheral = peripheral
        
        central.stopScan()
        peripheral.delegate = self
        central.connect(peripheral)
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        bluetoothStatus = "Connected to device. Discovering services..."
        isConnected = true
        peripheral.discoverServices([VESSEL_UUID])
    }
    
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: (any Error)?) {
        bluetoothStatus = "Failed to connect. Retrying..."
        startScan()
    }
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: (any Error)?) {
        bluetoothStatus = "Disconnected from device. Starting scan..."
        isConnected = false
        startScan()
    }
}
