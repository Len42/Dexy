using System;
using System.Collections.Generic;
using System.Data.SqlClient;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Utils
{
    /// <summary>
    /// Convert strings between Unicode and Latin1 encodings
    /// </summary>
    public static class ConvertString
    {
        /// <summary>
        /// Convert a Unicode-encoded string encoded to an array of bytes encoded
        /// as Latin1. (I know it says "Ascii", sue me.)
        /// </summary>
        /// <param name="strUnicode"></param>
        /// <returns></returns>
        public static byte[] ToAsciiBytes(string strUnicode)
        {
            byte[] unicodeBytes = Encoding.Unicode.GetBytes(strUnicode);
            return Encoding.Convert(Encoding.Unicode, Encoding.Latin1, unicodeBytes);
        }

        /// <summary>
        /// Convert an array of bytes representing a Latin1-encoded string
        /// to a Unicode-encoded string. (I know it says "Ascii", sue me.)
        /// </summary>
        /// <param name="asciiBytes"></param>
        /// <returns></returns>
        public static string ToString(byte[] asciiBytes)
        {
            char[] asciiChars = new char[Encoding.Latin1.GetCharCount(asciiBytes, 0, asciiBytes.Length)];
            Encoding.Latin1.GetChars(asciiBytes, 0, asciiBytes.Length, asciiChars, 0);
            return new string(asciiChars);
        }
    }
}
