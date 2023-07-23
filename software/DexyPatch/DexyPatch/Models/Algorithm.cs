using Avalonia.Controls.Templates;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;

namespace Dexy.DexyPatch.Models
{
    /// <summary>
    /// Definition of a synth "algorithm", i.e. how the 6 operators interact with each other
    /// </summary>
    /// <remarks>
    /// See AlgorithmData.cs for the definitions of all 32 algorithms
    /// </remarks>
    public partial struct Algorithm
    {
        public Algorithm(AlgoOp[] ops) { this.ops = ops; }

        /// <summary>
        /// List of operators
        /// </summary>
        public AlgoOp[] ops;
    }

    /// <summary>
    /// How modulation is used by an operator
    /// </summary>
    public enum UseMod
    {
        /// <summary>Not modulated</summary>
        none,
        /// <summary>Modulated by the previous operator</summary>
        prev,
        /// <summary>Modulated by the saved modulation value</summary>
        saved,
        /// <summary>Modulated by the feedback value</summary>
        fb
    }

    /// <summary>
    /// How an operator sets the saved modulation value
    /// </summary>
    public enum SaveMod
    {
        /// <summary>Do not save</summary>
        none,
        /// <summary>Save the modulation value</summary>
        set,
        /// <summary>Add to the saved modulation value</summary>
        add
    }

    /// <summary>
    /// Definition of what an operator does in an algorithm
    /// </summary>
    public struct AlgoOp
    {
        /// <summary>Is this operator an output (carrier) or a modulator?</summary>
        public bool isOutput;
        /// <summary>Type of modulation that this operator _uses_</summary>
        public UseMod mod;
        /// <summary>Does this operator _set_ the saved modulation value?</summary>
        public SaveMod saveMod;
        /// <summary>Does this operator _set_ the feedback value?</summary>
        public bool setsFb;
    }
}
