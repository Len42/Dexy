using System.IO;
using System.Text;

namespace Dexy.DexyPatch.Utils.Zpp
{
    /// <summary>
    /// Serialize objects in the format used by the zpp::bits C++ library.
    /// </summary>
    /// <remarks>
    /// I don't know whether or not System.Runtime.Serialization can be used to
    /// do this; figuring that out was much harder than just writing a simple
    /// implementation.
    /// </remarks>
    public static class ZppSerialize
    {
        /// <summary>
        /// Serialize an object into a stream
        /// </summary>
        /// <param name="str"></param>
        /// <param name="obj"></param>
        public static void Serialize(Stream str, IZppSerialize obj)
        {
            using var w = new ZppWriter(str, Encoding.Latin1);
            w.Write(serializeCookie);
            w.Write(serializeVersion);
            obj.Serialize(w);
        }

        /// <summary>
        /// Serialize a single byte value into a stream
        /// </summary>
        /// <param name="str"></param>
        /// <param name="b"></param>
        public static void Serialize(Stream str, byte b)
        {
            using var w = new ZppWriter(str, Encoding.Latin1);
            w.Write(serializeCookie);
            w.Write(serializeVersion);
            w.Write(b);
        }

        /// <summary>
        /// Serialize an object into a memory buffer
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        public static byte[] SerializeMem(IZppSerialize obj)
        {
            using var str = new MemoryStream();
            Serialize(str, obj);
            return str.ToArray();
        }

        /// <summary>
        /// Deserialize an object from a stream
        /// </summary>
        /// <param name="str"></param>
        /// <param name="obj"></param>
        /// <exception cref="Dexy.DexyPatch.Utils.Zpp.ZppException"></exception>
        public static void Deserialize(Stream str, IZppSerialize obj)
        {
            using var r = new ZppReader(str, Encoding.Latin1);
            uint cookie = r.ReadUInt32();
            if (cookie != serializeCookie)
            {
                throw new ZppException("Bad cookie");
            }
            ushort version = r.ReadUInt16();
            if (version != serializeVersion)
            {
                throw new ZppException("Version mismatch");
            }
            obj.Deserialize(r);
        }

        /// <summary>
        /// Deserialize an object from a memory buffer
        /// </summary>
        /// <param name="mem"></param>
        /// <param name="obj"></param>
        public static void DeserializeMem(byte[] mem, IZppSerialize obj)
        {
            using var str = new MemoryStream(mem);
            Deserialize(str, obj);
        }

        /// <summary>Magic cookie identifying serialized data</summary>
        /// <remarks>Little-endian</remarks>
        private const uint serializeCookie = 'D' | ('e' | ('x' | 'y' << 8) << 8) << 8;

        /// <summary>Version number of serialization data format</summary>
        private const ushort serializeVersion = 1;
    }
}
