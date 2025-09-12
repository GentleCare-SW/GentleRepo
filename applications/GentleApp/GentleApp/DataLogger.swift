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

import Foundation

public final class DataLogger {
    public enum DataLoggerError: Error, CustomStringConvertible {
        case valueCountMismatch(expected: Int, got: Int)
        case writeFailed

        public var description: String {
            switch self {
            case .valueCountMismatch(let expected, let got):
                return "Expected \(expected) values, but got \(got)."
            case .writeFailed:
                return "Failed to write to the CSV file."
            }
        }
    }
    
    private let columns: [String]
    private let fileURL: URL
    private var handle: FileHandle?
    private let queue = DispatchQueue(label: "DataLogger")

    private let iso8601: ISO8601DateFormatter = {
        let f = ISO8601DateFormatter()
        f.formatOptions = [.withInternetDateTime, .withFractionalSeconds]
        return f
    }()

    public init(columns: [String], fileURL: URL? = nil) throws {
        self.columns = columns

        let fm = FileManager.default
        if let fileURL = fileURL {
            self.fileURL = fileURL
        } else if let docs = fm.urls(for: .documentDirectory, in: .userDomainMask).first {
            self.fileURL = docs.appendingPathComponent("data.csv")
        } else {
            self.fileURL = fm.temporaryDirectory.appendingPathComponent("data.csv")
        }
        
        let exists = fm.fileExists(atPath: self.fileURL.path)
        if !exists {
            try "".write(to: self.fileURL, atomically: true, encoding: .utf8)
        }
        
        self.handle = try FileHandle(forWritingTo: self.fileURL)
        try self.handle?.seekToEnd()

        if !exists {
            try writeHeader()
        }
    }

    deinit {
        try? handle?.close()
    }

    public func logRow(_ values: [Float?]) throws {
        guard values.count == columns.count else {
            throw DataLoggerError.valueCountMismatch(expected: columns.count, got: values.count)
        }

        let timestamp = iso8601.string(from: Date())
        let formatted = values.map { String($0 ?? Float.nan) }

        let line = ([csvEscape(timestamp)] + formatted).joined(separator: ",") + "\n"
        try queue.sync {
            try appendString(line)
        }
    }

    private func writeHeader() throws {
        let header = (["timestamp"] + columns.map(csvEscape)).joined(separator: ",") + "\n"
        try appendString(header)
    }

    private func appendString(_ s: String) throws {
        guard let data = s.data(using: .utf8) else { throw DataLoggerError.writeFailed }
        try handle?.seekToEnd()
        try handle?.write(contentsOf: data)
    }

    private func csvEscape(_ s: String) -> String {
        if s.contains(where: { [",", "\"", "\n", "\r"].contains($0) }) {
            let escaped = s.replacingOccurrences(of: "\"", with: "\"\"")
            return "\"\(escaped)\""
        }
        return s
    }
}
