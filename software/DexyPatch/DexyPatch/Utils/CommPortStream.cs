using System;
using System.IO;

namespace Dexy.DexyPatch.Utils
{
    /// <summary>
    /// A <see cref="Stream"/> to read/write to/from a <see cref="Dexy.DexyPatch.Utils.CommPort"/>
    /// </summary>
    public class CommPortStream : Stream
    {
        public CommPortStream(CommPort portIn) => port = portIn;

        /// <summary>
        /// The comm port
        /// </summary>
        private readonly CommPort port;

        #region New methods

        /// <summary>
        /// The number of bytes available to read from the input buffer
        /// </summary>
        public int BytesAvail { get => port.BytesAvail; }

        /// <summary>
        /// Read the available text from the input buffer (without blocking)
        /// </summary>
        /// <returns></returns>
        public string ReadAvail() => port.ReadAvail();

        #endregion

        #region Overrides

        public override bool CanRead => true;

        public override bool CanWrite => true;

        public override bool CanSeek => false;

        public override long Length => throw new NotSupportedException();

        public override long Position { get => throw new NotSupportedException(); set => throw new NotSupportedException(); }

        public override void Flush() => throw new NotSupportedException();

        public override int Read(byte[] buffer, int offset, int count)
            => port.Read(buffer, offset, count);

        public override void Write(byte[] buffer, int offset, int count)
            => port.Write(buffer, offset, count);

        public override long Seek(long offset, SeekOrigin origin)
            => throw new NotSupportedException();

        public override void SetLength(long value)
            => throw new NotSupportedException();

        #endregion
    }
}
