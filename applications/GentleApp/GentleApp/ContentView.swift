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

import SwiftUI

struct ContentView: View {
    @StateObject private var vessel = RemoteVessel()

    var body: some View {
        Text("\(vessel.bluetoothStatus)")
        Text("Air Pressure (PSI): " + (vessel.airPressure == nil ? "N/A" : String(vessel.airPressure!)))
        Text("Voltage Percentage (%): " + (vessel.voltagePercentage == nil ? "N/A" : String(vessel.voltagePercentage!)))
        Text("Motor Position (rad): " + (vessel.motorPosition == nil ? "N/A" : String(vessel.motorPosition!)))
        Text("Motor Velocity (rad/s): " + (vessel.motorVelocity == nil ? "N/A" : String(vessel.motorVelocity!)))
        Text("Motor Torque (Nm): " + (vessel.motorTorque == nil ? "N/A" : String(vessel.motorTorque!)))
        Text("Servo Chamber: " + (vessel.servoChamber == nil ? "N/A" : String(vessel.servoChamber!)))
        Text("Vessel Mode: " + (vessel.mode == nil ? "N/A" : String(vessel.mode!.rawValue)))
        Button("Idle", action: { vessel.setMode(.idle) })
        Button("Evert", action: { vessel.setMode(.eversion) })
        Button("Evert Paused", action: { vessel.setMode(.eversionPaused) })
        Button("Invert", action: { vessel.setMode(.inversion) })
        Button("Invert Paused", action: { vessel.setMode(.inversionPaused) })
        Button("Chamber 0", action: { vessel.setChamber(0) })
        Button("Chamber 1", action: { vessel.setChamber(1) })
    }
}

#Preview {
    ContentView()
}
