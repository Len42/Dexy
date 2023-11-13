using System;
using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Dexy.DexyPatch.ViewModels;

namespace Dexy.DexyPatch
{
    /// <summary>
    /// <see cref="IDataTemplate"/> that creates a View corresponding to a ViewModel
    /// </summary>
    public class ViewLocator : IDataTemplate
    {
        public Control Build(object? data)
        {
            string name = "???";
            if (data != null) {
                name = data.GetType().FullName!.Replace("ViewModel", "View");
                var type = Type.GetType(name);
                if (type != null) {
                    return (Control)Activator.CreateInstance(type)!;
                }
            }
            return new TextBlock { Text = "Not Found: " + name };
        }

        public bool Match(object? data)
        {
            return data is ViewModelBase;
        }
    }
}