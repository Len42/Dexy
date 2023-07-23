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
        void Deserialize(ZppReader r);
    }
}
