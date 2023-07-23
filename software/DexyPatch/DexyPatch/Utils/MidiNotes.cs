using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Dexy.DexyPatch.Models;

namespace Dexy.DexyPatch.Utils
{
    /// <summary>
    /// Utilities for MIDI note numbers
    /// </summary>
    public static class MidiNotes
    {
        /// <summary>
        /// Return the frequency of the given MIDI note in Hz
        /// </summary>
        /// <param name="midiNote"></param>
        /// <returns></returns>
        public static double NoteToHz(Int16 midiNote)
        {
            return 440 * Math.Pow(2, ((double)midiNote / 256 - 69) / 12);
        }

        /// <summary>
        /// Return a formatted string with the frequency of the given MIDI note in Hz
        /// </summary>
        /// <param name="midiNote"></param>
        /// <returns></returns>
        public static string NoteToHzString(Int16 midiNote)
        {
            double hz = NoteToHz(midiNote);
            if (hz >= 1000) {
                return $"{hz:0.#} Hz";
            }else if (hz >= 10) {
                return $"{hz:0.##} Hz";
            } else {
                return $"{hz:0.###} Hz";
            }
        }

        /// <summary>
        /// Return a formatted string representing the given MIDI note, e.g. "F#4 +25"
        /// </summary>
        /// <param name="midiNote"></param>
        /// <returns></returns>
        public static string NoteToString(Int16 midiNote)
        {
            int noteNum = midiNote >> 8;
            int fraction = midiNote & 0xFF;
            if (fraction > 127) {
                ++noteNum;
                fraction -= 256;
            }
            int cents = (int)((double)fraction * 100 / DexyDefs.midiNoteSemitone);
            int octave = noteNum / 12 - 1;
            int note = noteNum % 12;
            string name = noteNames[note];
            return $"{name}{octave}{cents: +#; -#;''}";
        }

        /// <summary>
        /// List of note names
        /// </summary>
        private static readonly string[] noteNames = {
            "C","C♯","D","E♭","E","F","F♯","G","G♯","A","B♭","B"
        };
    }
}
