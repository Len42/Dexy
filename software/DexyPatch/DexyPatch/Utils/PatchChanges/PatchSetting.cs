using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Utils.PatchChanges
{
    /// <summary>
    /// Designates one of the settings that apply to the patch as a whole
    /// </summary>
    /// <seealso cref="Dexy.DexyPatch.Models.Patch"/>
    public enum PatchSetting : byte
    {
        /// <summary>Algorithm number</summary>
        algorithm,
        /// <summary>Feedback amount</summary>
        feedbackAmount,
        /// <summary>Patch name</summary>
        name
    }
}
