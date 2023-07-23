using System;
using System.Diagnostics;
using System.IO;
using System.IO.Ports;

namespace Dexy.DexyPatch.Utils
{
    /// <summary>
    /// Serial port interface to a Dexy device that automatically re-connects
    /// after temporary disconnections
    /// </summary>
    /// <remarks>
    /// Events signal connection and disconnection.
    /// Uses the Microsoft.Windows.Compatibility package.
    /// </remarks>
    public class CommPort : IDisposable
    {
        /// <summary>
        /// Settings to configure the serial port
        /// </summary>
        public class CommPortSettings
        {
            // These parameters must be set appropriately by the caller:

            /// <summary>Port name, e.g. "COM1"</summary>
            public string portName = "";
            /// <summary>Baud rate</summary>
            public int baudRate = 115200;
            /// <summary>Parity</summary>
            public Parity parity = Parity.None;
            /// <summary>Number of data bits</summary>
            public int dataBits = 8;
            /// <summary>Number of stop bits</summary>
            public StopBits stopBits = StopBits.One;

            // These parameters have suitable default values for this application:

            /// <summary>Handshaking protocol</summary>
            public Handshake handShake = Handshake.None;
            /// <summary>Enable DTR?</summary>
            public bool dtrEnable = true;
            /// <summary>Enable RTS?</summary>
            public bool rtsEnable = true;
            /// <summary>Read timeout in milliseconds</summary>
            public int readTimeout = 5000;
            /// <summary>Write timeout in milliseconds</summary>
            public int writeTimeout = 500;

            // TODO: ReadBufferSize & WriteBufferSize with suitable defaults
            // BUG: Can't serialize: public System.Text.Encoding encoding = System.Text.Encoding.Latin1;
        }

        public CommPort(CommPortSettings p)
        {
            port = new(p.portName, p.baudRate, p.parity, p.dataBits, p.stopBits) {
                Handshake = p.handShake,
                DtrEnable = p.dtrEnable,
                RtsEnable = p.rtsEnable,
                ReadTimeout = p.readTimeout,
                WriteTimeout = p.writeTimeout,
                Encoding = System.Text.Encoding.Latin1 // p.encoding
            };
            port.Open();
            SendCommPortConnected(true);
        }

        /// <summary>
        /// Low-level serial port interface
        /// </summary>
        private readonly SerialPort port;

        public void Dispose()
        {
            GC.SuppressFinalize(this);
            port.Dispose();
        }

        /// <summary>
        /// Is the serial port currently connected?
        /// </summary>
        public bool IsConnected => port.IsOpen;

        /// <summary>
        /// Close the serial port connection
        /// </summary>
        public void Close()
        {
            if (IsConnected) {
                port.Close();
                SendCommPortConnected(false);
            }
        }

        /// <summary>
        /// Sent when the Dexy device connects or disconnects
        /// </summary>
        public event CommPortConnectedHandler? CommPortConnected;

        /// <summary>
        /// Arguments for the <see cref="Dexy.DexyPatch.Utils.CommPort"/> event
        /// </summary>
        public class CommPortConnectedEventArgs
        {
            /// <summary>
            /// true: The device is connected; false: The device is disconnected
            /// </summary>
            public bool Connected { get; set; }
        }

        /// <summary>
        /// Method definition for the <see cref="Dexy.DexyPatch.Utils.CommPort"/> event
        /// </summary>
        public delegate void CommPortConnectedHandler(CommPortConnectedEventArgs e);

        // Need to keep track of the last event sent because it's hard to notice
        // when disconnections happen, and to avoid many redundant events. #kludge
        private bool lastConnected = false;

        /// <summary>
        /// Send a Connected event
        /// </summary>
        /// <param name="connected"></param>
        private void SendCommPortConnected(bool connected)
        {
            // Only send an event if the connection state has changed
            if (CommPortConnected != null && connected != lastConnected) {
                CommPortConnectedEventArgs e = new() { Connected = connected };
                CommPortConnected(e);
            }
            lastConnected = connected;
        }

        /// <summary>
        /// Perform an operation, with disconnection handling.
        /// </summary>
        /// <param name="op">The operation to perform.</param>
        /// <remarks>
        /// The operation is a callable with no arguments or return value
        /// (System.Action). Every operation has a different function signature,
        /// so args and retval are passed by variable capture if they are needed.
        /// While the port is disconnected, operations do nothing; e.g. Write
        /// doesn't write anything and doesn't indicate an error.
        /// Some classes that wrap a Stream (e.g. BinaryReader) cannot handle
        /// comm port disconnection. The application should check IsConnected
        /// before calling those classes, but disconnection errors are still
        /// possible in some situations.
        /// </remarks>
        private void DoOrDoNot(Action op)
        {
            // If the serial port was disconnected, try to reconnect.
            // If that fails just give up.
            // TODO: Fancier reconnection logic
            if (!IsConnected) {
                SendCommPortConnected(false); // in case this hasn't been noticed yet
                // BUG: Other exceptions seem to happen occasionally - maybe catch all?
                try {
                    port.Open();
                    SendCommPortConnected(true);
                } catch (FileNotFoundException) {
                    return;
                } catch (Exception ex) {
                    // DEBUG: Not sure if other exceptions can happen
                    Debug.WriteLine($"CommPort exception {ex.GetType()} {ex.Message}");
                    SendCommPortConnected(false);
                }
            }
            // Perform the operation. If it fails due to disconnection, return
            // with no error.
            // BUG: Other exceptions seem to happen occasionally - maybe catch all?
            try {
                op();
            } catch (OperationCanceledException) {
                SendCommPortConnected(false);
            } catch (Exception ex) {
                // DEBUG: Need to catch everything I think?
                Debug.WriteLine($"CommPort exception {ex.GetType()} {ex.Message}");
                SendCommPortConnected(false);
            }
        }

        /// <summary>
        /// The number of bytes available to read from the input buffer
        /// </summary>
        public int BytesAvail
        {
            get
            {
                int retVal = 0;
                DoOrDoNot(() => { retVal = port.BytesToRead; });
                return retVal;
            }
        }

        /// <summary>
        /// Read the available text from the input buffer (without blocking)
        /// </summary>
        /// <returns></returns>
        public string ReadAvail()
        {
            string retVal = String.Empty;
            DoOrDoNot(() => { retVal = port.ReadExisting(); });
            return retVal;
        }

        /// <summary>
        /// Read bytes from the serial port into a buffer
        /// </summary>
        /// <param name="buf">Buffer to read into</param>
        /// <param name="offset">Offset in buffer to start reading into</param>
        /// <param name="count">Number of bytes to read</param>
        /// <returns></returns>
        public int Read(byte[] buf, int offset, int count)
        {
            int retVal = 0;
            DoOrDoNot(() => { retVal = port.Read(buf, offset, count); });
            return retVal;
        }

        /// <summary>
        /// Write data to the serial port
        /// </summary>
        /// <param name="buf">Buffer containing the data to be written</param>
        public void Write(byte[] buf) { DoOrDoNot(() => { Write(buf, 0, buf.Length); }); }

        /// <summary>
        /// Write data to the serial port
        /// </summary>
        /// <param name="buf">Buffer containing the data to be written</param>
        /// <param name="offset">Offset in buffer of the start of the data to write</param>
        /// <param name="count">Number of bytes to write</param>
        public void Write(byte[] buf, int offset, int count) { DoOrDoNot(() => { port.Write(buf, offset, count); }); }

        /// <summary>
        /// Write a string to the serial port
        /// </summary>
        /// <param name="buf">The string to write</param>
        public void Write(string buf) { DoOrDoNot(() => { port.Write(buf); }); }

        /// <summary>
        /// Get a list of the names of all the serial ports
        /// </summary>
        /// <returns></returns>
        public static string[] GetPortNames() { return SerialPort.GetPortNames(); }
    }
}
