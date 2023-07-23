using Dexy.DexyPatch.Utils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Services
{
    /// <summary>
    /// Service for saving and loading app configuration settings
    /// </summary>
    public class SettingsManager : ISettingsManager
    {
        public SettingsManager()
        {
            settingsDir = SysUtils.AppDataFolder;
            if (!Directory.Exists(settingsDir)) {
                Directory.CreateDirectory(settingsDir);
            }
        }

        private readonly string settingsDir;

        public string SettingsDir { get => settingsDir; }

        private static readonly JsonSerializerOptions options = new() { IncludeFields = true };

        public T Load<T>() where T : class?, new()
        {
            T? settings = null;
            string filePath = Path.Combine(settingsDir, typeof(T).Name);
            if (File.Exists(filePath)) {
                using var stream = File.Open(filePath, FileMode.Open, FileAccess.Read);
                settings = JsonSerializer.Deserialize<T>(stream, options);
            }
            return settings ?? new();
        }

        public void Save<T>(T settings) where T : class?
        {
            string filePath = Path.Combine(settingsDir, typeof(T).Name);
            using var stream = File.Open(filePath, FileMode.Create, FileAccess.Write);
            JsonSerializer.Serialize(stream, settings, options);
        }

        public void Terminate() { }
    }
}
