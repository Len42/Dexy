using System.IO;
using System.Text;

namespace Dexy.DexyPatch.Utils.Zpp
{
    /// <summary>
    /// Subclass of <see cref="BinaryReader"/> with some helpful methods
    /// </summary>
    public class ZppReader : BinaryReader
    {
        public ZppReader(Stream input, Encoding encoding) : base(input, encoding) { }

        /// <summary>
        /// Read an object that supports <see cref="Dexy.DexyPatch.Utils.Zpp.IZppSerialize"/>
        /// </summary>
        /// <param name="obj"></param>
        public void Read(IZppSerialize obj) { obj.Deserialize(this); }

        /// <summary>
        /// Read a fixed length array of objects
        /// </summary>
        /// <param name="arr"></param>
        /// <param name="size"></param>
        /// <exception cref="Dexy.DexyPatch.Utils.Zpp.ZppException"></exception>
        public void ReadArray(IZppSerialize[] arr, uint size)
        {
            if (arr.Length != size) {
                throw new ZppException("Argument size mismatch");
            }
            for (int i = 0; i < arr.Length; i++) {
                arr[i].Deserialize(this);
            }
        }

        /// <summary>
        /// Read a fixed-length string
        /// </summary>
        /// <param name="size"></param>
        /// <returns></returns>
        public string ReadFixedLength(uint size)
        {
            StringBuilder sb = new((int)size);
            for (int i = 0; i < size; i++) {
                sb.Append(ReadChar());
            }
            return sb.ToString();
        }
    }
}
