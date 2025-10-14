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

enum PlatformMode: Float {
    case idle = 0.0
    case eversion = 1.0
    case eversionPaused = 2.0
    case inversion = 3.0
    case inversionPaused = 4.0
}

class RemotePlatform: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    @Published var bluetoothStatus: String = "Starting..."
    @Published var isConnected: Bool = false
    @Published var connectedDeviceName: String = ""
    @Published var availableDevices: [String] = []

    @Published var airPressure: Float?
    @Published var pressureSensorError: Float?
    @Published var voltage: Float?
    @Published var motorPosition: Float?
    @Published var motorVelocity: Float?
    @Published var motorTorque: Float?
    @Published var motorError: Float?
    @Published var servoChamber: Float?
    @Published var mode: PlatformMode?
    @Published var progress: Float?
    
    private var availablePeripherals: [String:CBPeripheral] = [:]
    private var pollTimer: Timer!
    private var central: CBCentralManager!
    private var peripheral: CBPeripheral?
    private var characteristics: [CBUUID: CBCharacteristic] = [:]
    private var values: [CBUUID: Float] = [:]
    private var dataLogger: DataLogger!
    
    override init() {
        super.init()
        central = CBCentralManager(delegate: self, queue: nil)
        pollTimer = Timer.scheduledTimer(withTimeInterval: 0.1, repeats: true, block: { [weak self] _ in self?.poll() })
        dataLogger = try! DataLogger(columns: CHARACTERISTIC_UUIDS.map { $0.uuidString })
    }
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            bluetoothStatus = "Searching for device..."
            startScan()
        case .poweredOff:
            bluetoothStatus = "Bluetooth is off. Please enable it in Settings."
        case .unauthorized:
            bluetoothStatus = "No Bluetooth permission. Grant it in Settings."
        case .unsupported:
            bluetoothStatus = "This device doesn't support Bluetooth."
        case .resetting:
            bluetoothStatus = "Bluetooth resetting..."
        default:
            bluetoothStatus = "Unknown error."
        }
    }
    
    private func startScan() {
        if central.state != .poweredOn {
            return;
        }
        
        availableDevices = []
        availablePeripherals = [:]
        central.scanForPeripherals(withServices: [SERVICE_UUID], options: [CBCentralManagerScanOptionAllowDuplicatesKey: true])
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if peripheral.name == nil {
            return
        }
        
        availablePeripherals[peripheral.name!] = peripheral
        availableDevices = availablePeripherals.keys.sorted()
    }
    
    func connectToDevice(_ deviceName: String) {
        if availablePeripherals[deviceName] == nil {
            return
        }
        
        bluetoothStatus = "Connecting..."
        central.stopScan()
        peripheral = availablePeripherals[deviceName]!
        peripheral!.delegate = self
        central.connect(availablePeripherals[deviceName]!)
    }
    
    func disconnectFromDevice() {
        if !isConnected {
            return
        }
        
        bluetoothStatus = "Disconnected from device. Searching..."
        isConnected = false
        central.cancelPeripheralConnection(peripheral!)
        startScan()
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        bluetoothStatus = "Connected to device. Discovering services..."
        peripheral.discoverServices([SERVICE_UUID])
    }
    
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: (any Error)?) {
        bluetoothStatus = "Failed to connect. Retrying..."
        startScan()
    }
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: (any Error)?) {
        bluetoothStatus = "Disconnected from device. Searching..."
        isConnected = false
        startScan()
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: (any Error)?) {
        if error != nil || peripheral.services == nil {
            bluetoothStatus = "Service discovery error."
            return
        }
        
        bluetoothStatus = "Discovered services. Discovering characteristics..."
        peripheral.discoverCharacteristics(CHARACTERISTIC_UUIDS, for: peripheral.services![0])
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: (any Error)?) {
        if error != nil || peripheral.services![0].characteristics == nil {
            bluetoothStatus = "Characteristic discovery error."
            return
        }
        
        for characteristic in peripheral.services![0].characteristics! {
            characteristics[characteristic.uuid] = characteristic
            if characteristic.properties.contains(.notify) {
                peripheral.setNotifyValue(true, for: characteristic)
            }
        }
        
        bluetoothStatus = "Connected to device."
        isConnected = true
        connectedDeviceName = peripheral.name!
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: (any Error)?) {
        if error != nil || characteristic.value == nil {
            bluetoothStatus = "Failed to read value."
            return
        }
        
        if characteristic.value!.count != 4 {
            return
        }
        
        let value = Float(bitPattern: UInt32(littleEndian: characteristic.value!.withUnsafeBytes { $0.load(as: UInt32.self) }))
        values[characteristic.uuid] = value
    }
    
    private func poll() {
        if !isConnected {
            return
        }
        
        try! dataLogger.logRow(CHARACTERISTIC_UUIDS.map { values[$0] })
        
        airPressure = values[PRESSURE_SENSOR_UUID]
        pressureSensorError = values[PRESSURE_SENSOR_ERROR_UUID]
        voltage = values[DIMMER_VOLTAGE_UUID]
        motorPosition = values[MOTOR_POSITION_UUID]
        motorVelocity = values[MOTOR_VELOCITY_UUID]
        motorTorque = values[MOTOR_TORQUE_UUID]
        motorError = values[MOTOR_ERROR_UUID]
        servoChamber = values[SERVO_CHAMBER_UUID]
        mode = values[AUTO_CONTROL_MODE_UUID] == nil ? nil : PlatformMode(rawValue: values[AUTO_CONTROL_MODE_UUID]!)
        progress = values[AUTO_CONTROL_PROGRESS_UUID]
    }
    
    private func writeValue(_ value: Float, for uuid: CBUUID) {
        if peripheral == nil || characteristics[uuid] == nil {
            return
        }
        
        var bitPattern = value.bitPattern.littleEndian
        let data = withUnsafeBytes(of: &bitPattern) { Data($0) }
        peripheral!.writeValue(data, for: characteristics[uuid]!, type: .withResponse)
    }
    
    public func setMode(_ mode: PlatformMode) {
        writeValue(mode.rawValue, for: AUTO_CONTROL_MODE_UUID)
    }
    
    public func setChamber(_ chamber: Int) {
        writeValue(Float(chamber), for: SERVO_CHAMBER_UUID)
    }
    
    public func setVoltage(_ voltage: Float) {
        writeValue(voltage, for: DIMMER_VOLTAGE_UUID)
    }
    
    public func setPressureSetpoint(_ pressure: Float) {
        writeValue(pressure, for: PRESSURE_CONTROLLER_UUID)
    }
    
    public func setMotorVelocity(_ velocity: Float) {
        writeValue(velocity, for: MOTOR_VELOCITY_UUID)
    }
}
