using System.IO;
using System.Text;

namespace Dexy.DexyPatch.Utils.Zpp
{
    /// <summary>
    /// Subclass of <see cref="BinaryWriter"/> with some helpful methods
    /// </summary>
    public class ZppWriter : BinaryWriter
    {
        public ZppWriter(Stream output, Encoding encoding) : base(output, encoding) { }

        /// <summary>
        /// Write an object that supports <see cref="Dexy.DexyPatch.Utils.Zpp.IZppSerialize"/>
        /// </summary>
        /// <param name="obj"></param>
        public void Write(IZppSerialize obj) { obj.Serialize(this); }

        /// <summary>
        /// Write a fixed length array of objects
        /// </summary>
        /// <param name="arr"></param>
        /// <param name="size"></param>
        /// <exception cref="Dexy.DexyPatch.Utils.Zpp.ZppException"></exception>
        public void WriteArray(IZppSerialize[] arr, uint size)
        {
            if (arr.Length != size) {
                throw new ZppException("Argument size mismatch");
            }
            foreach (IZppSerialize? obj in arr) {
                obj?.Serialize(this);
            }
        }

        /// <summary>
        /// Write a fixed length string, padding it with spaces if necessary
        /// </summary>
        /// <param name="str"></param>
        /// <param name="size"></param>
        /// <exception cref="Dexy.DexyPatch.Utils.Zpp.ZppException"></exception>
        public void WriteFixedLength(string str, uint size)
        {
            if (str.Length != size) {
                throw new ZppException("Argument size mismatch");
            }
            WriteFixedLength(str, size, ' ');
        }

        /// <summary>
        /// Write a fixed length string, padding it if necessary
        /// </summary>
        /// <param name="str"></param>
        /// <param name="size"></param>
        /// <param name="chPad"></param>
        /// <exception cref="Dexy.DexyPatch.Utils.Zpp.ZppException"></exception>
        public void WriteFixedLength(string str, uint size, char chPad)
        {
            if (str.Length > size) {
                throw new ZppException("Argument size mismatch");
            }
            foreach (char ch in str) {
                Write(ch);
            }
            for (uint i = (uint)str.Length; i < size; ++i) {
                Write(chPad);
            }
        }
    }
}
