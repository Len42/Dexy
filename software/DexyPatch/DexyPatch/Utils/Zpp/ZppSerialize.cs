using System.IO;
using System.Text;

namespace Dexy.DexyPatch.Utils.Zpp
{
    /// <summary>
    /// Serialize objects in the format used by the zpp::bits C++ library.
    /// </summary>
    /// <remarks>
    /// System.Runtime.Serialization is not used because I couldn't figure out
    /// how to make it handle the data format defined by zpp::bits, which is
    /// used by the module firmware.
    /// 
    /// Versioning
    /// ----------
    /// <see cref="Dexy.DexyPatch.Utils.Zpp.ZppSerialize.serializeVersion"/>
    /// defines the version of the data format for serialization. It must be
    /// incremented whenever the data format is changed.
    /// 
    /// <see cref="Dexy.DexyPatch.Utils.Zpp.ZppSerialize.Serialize"/> writes
    /// data in the latest format (specified by serialVersion).
    /// 
    /// <see cref="Dexy.DexyPatch.Utils.Zpp.ZppSerialize.Deserialize"/> can read
    /// data in any format back to version 1.
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
            if (cookie != serializeCookie) {
                throw new ZppException("Bad cookie");
            }
            ushort version = r.ReadUInt16();
            if (version < 1 || version > serializeVersion) {
                throw new ZppException("Version mismatch");
            }
            obj.Deserialize(r, version);
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
        /// <remarks>Must be incremented when the data format is changed.</remarks>
        private const ushort serializeVersion = 2;
    }
}
