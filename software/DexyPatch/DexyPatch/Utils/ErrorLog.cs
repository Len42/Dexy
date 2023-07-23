using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Utils
{
    /// <summary>
    /// Log (unhandled) exception to a file
    /// </summary>
    public static class ErrorLog
    {
        /// <summary>
        /// Log an exception to a file
        /// </summary>
        /// <param name="ex"></param>
        public static void WriteError(Exception ex)
        {
            try {
                // TODO: option to append or erase the file
                using var stream = File.Open(FileName, FileMode.Create, FileAccess.Write);
                using var writer = new StreamWriter(stream);
                writer.WriteLine($"{DateTime.Now} {ProjectVersion.AppName} {ProjectVersion.VersionLong}");
                WriteException(writer, null, ex);
                if(ex.InnerException != null) {
                    WriteException(writer, "OTHER ERROR", ex.InnerException);
                }
            } catch { }
        }

        /// <summary>
        /// File pathname to log exceptions
        /// </summary>
        private static string FileName { get => Path.Combine(SysUtils.AppDataFolder, "ErrorLog.txt"); }

        /// <summary>
        /// Write an exception to the log file
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
