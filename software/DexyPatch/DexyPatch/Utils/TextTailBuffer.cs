using System;
using System.Diagnostics;
using System.Text;

namespace Dexy.DexyPatch.Utils
{
    /// <summary>
    /// Save the most recent portion of a text stream in a text buffer
    /// </summary>
    public class TextTailBuffer
    {
        public TextTailBuffer() : this(defaultCapacity) { }

        public TextTailBuffer(int capacity) { stringBuilder = new StringBuilder(0, capacity); }

        /// <summary>Default capacity of the buffer</summary>
        private const int defaultCapacity = 4000;

        /// <summary>The text buffer</summary>
        private readonly StringBuilder stringBuilder;

        /// <summary>
        /// Return a string representing the object, which is simply the
        /// contents of the text buffer.
        /// </summary>
        /// <returns>The contents of the text buffer</returns>
        public override string ToString() => stringBuilder.ToString();

        /// <summary>Delete the contents of text buffer</summary>
        public void Clear() { stringBuilder.Clear(); }

        /// <summary>
        /// Append a string to the _end_ of the text buffer.
        /// If the buffer overflows, delete the text at the _start_ of the buffer
        /// so that the buffer always contains the most recent text that was added.
        /// </summary>
        /// <param name="s"></param>
        public void Append(string s)
        {
            int len = s.Length;
            if (len > stringBuilder.MaxCapacity) {
                s = s[(len - stringBuilder.MaxCapacity)..];
                len = stringBuilder.MaxCapacity;
            }
            int required = stringBuilder.Length + len;
            if (required > stringBuilder.MaxCapacity) {
                stringBuilder.Remove(0, required - stringBuilder.MaxCapacity);
            }
            stringBuilder.Append(s);
        }
    } 
}
