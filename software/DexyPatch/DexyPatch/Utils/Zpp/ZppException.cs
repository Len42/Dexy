using System;

namespace Dexy.DexyPatch.Utils.Zpp
{
    /// <summary>
    /// Exception thrown when an error occurs during serialization or deserialization
    /// </summary>
    public class ZppException : Exception
    {
        public ZppException() : base("Bad patch data") { }

        public ZppException(string message)
            : base($"Bad patch data: {message}") { }

        public ZppException(string message, Exception innerException)
            : base($"Bad patch data: {message}", innerException) { }
    }
}
