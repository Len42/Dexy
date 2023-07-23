using System;
using System.Linq;
using Dexy.DexyPatch.Utils.Zpp;

namespace Dexy.DexyPatch.Models
{
    /// <summary>
    /// Patch settings
    /// </summary>
    /// <remarks>
    /// Must match the corresponding C++ class
    /// </remarks>
    public class Patch : IZppSerialize
    {
        /// <summary>Settings for the operators</summary>
        public OpParams[] opParams;
        /// <summary>Algorithm number</summary>
        public byte algorithm;
        /// <summary>Feedback amount</summary>
        public UInt16 feedbackAmount;
        /// <summary>Patch name, up to maxName characters</summary>
        public string name;

        /// <summary>
        /// Maximum patch name length
        /// </summary>
        public const int patchNameLen = 16;

        public Patch()
        {
            opParams = new OpParams[DexyDefs.numOperators];
            for (int i = 0; i < DexyDefs.numOperators; i++) {
                opParams[i] = new();
            }
            algorithm = 0;
            feedbackAmount = DexyDefs.max_param_t;
            name = new string('?', patchNameLen); // DEBUG: should be ' '
        }

        /// <summary>
        /// Are all the settings in this object OK?
        /// </summary>
        /// <returns></returns>
        public bool IsValid()
        {
            return algorithm < DexyDefs.numAlgorithms
                && feedbackAmount <= DexyDefs.max_param_t
                && name.Length == patchNameLen
                && name.All((ch) => (ch >= ' ' && ch <= (char)255)) // Any character >= ' ' can be displayed
                && opParams.Length == DexyDefs.numOperators
                && opParams.All((op) => op.IsValid());
        }

        /// <summary>
        /// Return a copy of this object
        /// </summary>
        /// <returns></returns>
        public Patch Clone()
        {
            Patch copy = (Patch)MemberwiseClone();
            copy.opParams = (OpParams[])opParams.Clone();
            for (int i = 0; i < DexyDefs.numOperators; i++) {
                copy.opParams[i] = opParams[i].Clone();
            }
            return copy;
        }

        #region IZppSerialize interface
    
        public void Serialize(ZppWriter w)
        {
            w.WriteArray(opParams, DexyDefs.numOperators);
            w.Write(algorithm);
            w.Write(feedbackAmount);
            w.WriteFixedLength(name, patchNameLen);
        }

        public void Deserialize(ZppReader r)
        {
            r.ReadArray(opParams, DexyDefs.numOperators);
            algorithm = r.ReadByte();
            feedbackAmount = r.ReadUInt16();
            name = r.ReadFixedLength(patchNameLen);
        }

        #endregion

        #region Static methods

        /// <summary>
        /// Return a default Patch
        /// </summary>
        /// <returns></returns>
        public static Patch MakeDefaultPatch()
        {
            Patch patch = new() {
                algorithm = 15,
                feedbackAmount = DexyDefs.max_param_t,
                name = "défaut".PadRight(patchNameLen, ' ')
            };
            for (int i = 0; i < DexyDefs.numOperators - 1; i++) {
                patch.opParams[i] = new() {
                    fixedFreq = false,
                    noteOrFreq = 2 * DexyDefs.freqRatio1,
                    outputLevel = 700,
                    useEnvelope = true,
                    env = new() {
                        delay = 0,
                        attack = 930,
                        decay = 500,
                        sustain = 700,
                        release = 400,
                        loop = false
                    },
                    ampModSens = DexyDefs.max_param_t
                };
            }
            // Different settings for the last carrier operator
            patch.opParams[DexyDefs.numOperators - 1] = new() {
                fixedFreq = false,
                noteOrFreq = DexyDefs.freqRatio1,
                outputLevel = DexyDefs.max_param_t,
                useEnvelope = true,
                env = new() {
                    delay = 0,
                    attack = 930,
                    decay = 400,
                    sustain = 930,
                    release = 400,
                    loop = false
                },
                ampModSens = 0
            };
            return patch;
        }

        /// <summary>
        /// Return a bell-like patch
        /// </summary>
        /// <returns></returns>
        public static Patch MakeBellPatch()
        {
            Patch patch = new() {
                algorithm = 4,
                feedbackAmount = DexyDefs.max_param_t,
                name = "bells".PadRight(patchNameLen, ' ')
            };
            // carrier
            patch.opParams[5] = new() {
                fixedFreq = false,
                noteOrFreq = (ushort)(1.01 * DexyDefs.freqRatio1),
                outputLevel = 972,
                useEnvelope = true,
                env = new() {
                    delay = 0,
                    attack = 1000,
                    decay = 300,
                    sustain = 0,
                    release = 300,
                    loop = false
                },
                ampModSens = 0
            };
            // mod
            patch.opParams[4] = new() {
                fixedFreq = false,
                noteOrFreq = (ushort)(3.51 * DexyDefs.freqRatio1),
                outputLevel = 700,
                useEnvelope = true,
                env = new() {
                    delay = 0,
                    attack = 1020,
                    decay = 120,
                    sustain = 0,
                    release = 200,
                    loop = false
                },
                ampModSens = 0
            };
            // carrier
            patch.opParams[3] = new() {
                fixedFreq = false,
                noteOrFreq = (ushort)(0.99 * DexyDefs.freqRatio1),
                outputLevel = DexyDefs.max_param_t,
                useEnvelope = true,
                env = new() {
                    delay = 0,
                    attack = 1000,
                    decay = 300,
                    sustain = 0,
                    release = 300,
                    loop = false
                },
                ampModSens = 0
            };
            // mod
            patch.opParams[2] = new() {
                fixedFreq = false,
                noteOrFreq = (ushort)(3.49 * DexyDefs.freqRatio1),
                outputLevel = 670,
                useEnvelope = true,
                env = new() {
                    delay = 0,
                    attack = 1020,
                    decay = 120,
                    sustain = 0,
                    release = 200,
                    loop = false
                },
                ampModSens = 0
            };
            // carrier
            patch.opParams[1] = new() {
                fixedFreq = true,
                noteOrFreq = (ushort)(64 * DexyDefs.midiNoteSemitone),
                outputLevel = DexyDefs.max_param_t,
                useEnvelope = true,
                env = new() {
                    delay = 0,
                    attack = 700,
                    decay = 500,
                    sustain = 0,
                    release = 600,
                    loop = false
                },
                ampModSens = 0
            };
            // mod, feedback
            patch.opParams[0] = new() {
                fixedFreq = false,
                noteOrFreq = (ushort)(1.99 * DexyDefs.freqRatio1),
                outputLevel = 800,
                useEnvelope = true,
                env = new() {
                    delay = 0,
                    attack = 1020,
                    decay = 800,
                    sustain = 0,
                    release = 910,
                    loop = false
                },
                ampModSens = 0
            };
            return patch;
        }

        /// <summary>
        /// Return a sine wave patch
        /// </summary>
        /// <returns></returns>
        public static Patch MakeSinePatch()
        {
            Patch patch = new() {
                algorithm = 31,
                feedbackAmount = 0,
                name = "sine".PadRight(patchNameLen, ' ')
            };
            for (int i = 0; i < DexyDefs.numOperators - 1; i++) {
                patch.opParams[i] = new() {
                    fixedFreq = false,
                    noteOrFreq = DexyDefs.freqRatio1,
                    outputLevel = 0,
                    useEnvelope = true,
                    env = new() {
                        delay = 0,
                        attack = 930,
                        decay = 400,
                        sustain = 930,
                        release = 400,
                        loop = false
                    },
                    ampModSens = 0
                };
            }
            // Different settings for the last carrier operator
            patch.opParams[DexyDefs.numOperators - 1] = new() {
                fixedFreq = false,
                noteOrFreq = DexyDefs.freqRatio1,
                outputLevel = DexyDefs.max_param_t,
                useEnvelope = true,
                env = new() {
                    delay = 0,
                    attack = 900,
                    decay = 0,
                    sustain = DexyDefs.max_param_t,
                    release = 400,
                    loop = false
                },
                ampModSens = 0
            };
            return patch;
        }

        #endregion
    }
}
