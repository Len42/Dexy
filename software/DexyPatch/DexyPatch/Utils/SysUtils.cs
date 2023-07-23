using Avalonia;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace Dexy.DexyPatch.Utils
{
    /// <summary>
    /// System-specific utility functions
    /// </summary>
    public static class SysUtils
    {
        /// <summary>
        /// Return the directory where configuration settings will be stored.
        /// </summary>
        /// <remarks>
        /// Environment.GetFolderPath is not portable, hence this.
        /// Thanks to dotNetDave https://dotnettips.wordpress.com/2017/02/20/net-framework-core-getting-the-app-data-folder/
        /// </remarks>
        public static string AppDataFolder
        {
            get
            {
                var userPath = Environment.GetEnvironmentVariable(
                        RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
                            ? "LOCALAPPDATA" : "Home") ?? "~";
                //var assy = System.Reflection.Assembly.GetEntryAssembly();
                //var company = assy?.GetCustomAttributes<AssemblyCompanyAttribute>().FirstOrDefault();
                //var path = System.IO.Path.Combine(userPath, company?.Company ?? "?");
                var path = System.IO.Path.Combine(userPath, ProjectVersion.AppName);
                return path;
            }
        }

        /// <summary>
        /// Abbreviate a pathname to a maximum length, replacing the mid-section
        /// of the pathname with "..." if necessary.
        /// </summary>
        /// <param name="pathname"></param>
        /// <param name="maxChars"></param>
        /// <returns></returns>
        public static string AbbreviatePathname(string pathname, int maxChars)
        {
            const char chEllipsis = '…';
            if (pathname.Length <= maxChars) {
                return pathname;
            } else if (maxChars < 1) {
                return "";
            } else if (maxChars == 1) {
                return chEllipsis.ToString();
            } else {
                string filename = Path.GetFileName(pathname);
                if (filename.Length > maxChars - 1) {
                    return chEllipsis + filename[..(maxChars - 2)] + chEllipsis;
                } else {
                    // TODO: Instead of removing some characters from the middle,
                    // remove complete directory components from the middle.
                    int endLength = filename.Length + (maxChars - filename.Length - 1) / 2;
                    int startLength = maxChars - 1 - endLength;
                    return pathname[..startLength] + chEllipsis + pathname[(pathname.Length-endLength)..];
                }
            }
        }

        /// <summary>
        /// Get the name of the psudo-drive representing a Dexy device in bootloader mode
        /// </summary>
        /// <returns>Pathname to the root directory of the Dexy device drive</returns>
        public static string? FindDexyDrive()
        {
            string? drivePath = null;
            foreach (var drive in DriveInfo.GetDrives()) {
                if (drive.IsReady && drive.VolumeLabel == "RPI-RP2") {
                    // DriveInfo.Name is documented as "the name of a drive" but
                    // it's actually a pathname to the root directory of the drive.
                    drivePath = drive.Name;
                    break;
                }
            }
            return drivePath;
        }
    }
}
