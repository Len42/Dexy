namespace Dexy.DexyPatch.Utils.Zpp
{
    /// <summary>
    /// Interface to be implemented by objects that support ZppSerialize serialization
    /// </summary>
    public interface IZppSerialize
    {
        /// <summary>
        /// Write this object's data
        /// </summary>
        /// <param name="w"></param>
        void Serialize(ZppWriter w);

        /// <summary>
        /// Read this object's data
        /// </summary>
        /// <param name="r"></param>
        /// <param name="version">Data format version number</param>
        /// <remarks>
        /// Any implementation must be able to handle any data format back to
        /// <paramref name="version"/> == 1, filling in default values for any
        /// fields not included in the old data format.
        /// </remarks>
        void Deserialize(ZppReader r, ushort version);
    }
}
