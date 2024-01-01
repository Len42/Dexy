using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Utils
{
    /// <summary>
    /// Log an exception to a file or to the debug trace log
    /// </summary>
    public static class ErrorLog
    {
        /// <summary>
        /// Log an error message for an exception to a file
        /// </summary>
        /// <param name="ex"></param>
        public static void WriteError(Exception ex)
        {
            try {
                // TODO: option to append or erase the file
                using var stream = File.Open(FileName, FileMode.Create, FileAccess.Write);
                using var writer = new StreamWriter(stream);
                WriteError(writer, ex);
            } catch { }
        }

        /// <summary>
        /// Write an error message for an exception to the debug trace log
        /// </summary>
        /// <param name="ex"></param>
        public static void TraceError(Exception ex)
        {
            try {
                var writer = new StringWriter();
                WriteError(writer, ex);
                Trace.TraceError(writer.ToString());
            }
            catch { }
        }

        /// <summary>
        /// Write an error message to the debug trace log
        /// </summary>
        /// <param name="st"></param>
        public static void TraceError(string st)
        {
            Trace.TraceError(st);
        }

        /// <summary>
        /// Write a message to the debug trace log
        /// </summary>
        /// <param name="st"></param>
        public static void TraceMessage(string st)
        {
            Trace.TraceInformation(st);
        }

        /// <summary>
        /// File pathname to log exceptions
        /// </summary>
        private static string FileName { get => Path.Combine(SysUtils.AppDataFolder, "ErrorLog.txt"); }

        /// <summary>
        /// Write an error message for an exception to a TextWriter
        /// </summary>
        /// <param name="writer"></param>
        /// <param name="ex"></param>
        private static void WriteError(TextWriter writer, Exception ex)
        {
            writer.WriteLine($"{DateTime.Now} {ProjectVersion.AppName} {ProjectVersion.VersionLong}");
            WriteException(writer, null, ex);
            if (ex.InnerException != null)
            {
                WriteException(writer, "OTHER ERROR", ex.InnerException);
            }
        }

        /// <summary>
        /// Write a single exception to the log file
        /// </summary>
        /// <param name="writer"></param>
        /// <param name="stMessage"></param>
        /// <param name="ex"></param>
        private static void WriteException(TextWriter writer, string? stMessage, Exception ex)
        {
            if(stMessage != null) {
                writer.Write($"{stMessage}: ");
            }
            writer.WriteLine($"{ex.Message} ({ex.GetType().Name})");
            writer.WriteLine(ex.StackTrace);
        }
    }
}
