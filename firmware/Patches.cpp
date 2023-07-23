namespace Dexy { namespace Patches {

/// @brief Is the given char valid for a patch name?
/// @details This is based on the set of characters that can be displayed.
/// @param ch 
/// @return Yes or no
static constexpr bool isGoodChar(char ch)
{
    // Any character >= ' ' can be displayed.
    // Don't allow null char. Patch name field must be blank-padded not 0-terminated.
    constexpr char charMin = 32;
    return ch >= charMin;
}

constexpr bool EnvParams::isValid() const
{
    return delay <= max_param_t
        && attack <= max_param_t
        && decay <= max_param_t
        && sustain <= max_param_t
        && release <= max_param_t;
}

constexpr bool OpParams::isValid() const
{
    return outputLevel <= max_param_t
        && ampModSens <= max_param_t
        && env.isValid();
}

constexpr bool Patch::isValid() const
{
    return algorithm < numAlgorithms
        && feedbackAmount <= max_param_t
        && std::ranges::all_of(name, [](auto&& ch){return isGoodChar(ch);})
        && std::ranges::all_of(opParams, [](auto&& op) { return op.isValid(); });
}

constexpr bool PatchBank::isValid() const
{
    return std::ranges::all_of(patches, [](auto&& patch){return patch.isValid();});
}

/// @brief Short consteval helper to serialize a PatchBank for initialization
/// @tparam patchBank 
/// @return 
template<PatchBank patchBank>
static consteval SerializedPatchBank patchBankToBytes()
{
    return Serialize::objToBytes<PatchBank, patchBank, Serialize::patchBankDataSize>();
}

/// @brief The initial patch data that is loaded into the PatchBank at startup
/// @details Stored in flash memory with alignment and padding so it can be
/// over-written by the SDK flash programming API.
IN_FLASH("PatchData")
static constinit Flash::Wrapper<SerializedPatchBank> initialPatchData {
    patchBankToBytes<makeDefaultPatchBank()>() // #embed pls?
};

/// @brief The patch bank that is currently loaded
static PatchBank patchBankCurrent;

/// @brief Compile-time consistency checks
static consteval void verifyData()
{
    static_assert(makeDefaultPatchBank().isValid());
    static_assert(Serialize::patchBankDataSize == patchBankToBytes<makeDefaultPatchBank()>().size());
    static_assert((sizeof(Flash::Wrapper<SerializedPatchBank>) % 4096) == 0);
    constexpr PatchChange p{.iPatch=0, .patch={}};
    static_assert(Serialize::patchChangeDataSize == Serialize::objToBytes<PatchChange, p, Serialize::patchChangeDataSize>().size());
    constexpr PatchNameChange name{.iPatch=0, .name=""};
    static_assert(Serialize::patchNameChangeDataSize == Serialize::objToBytes<PatchNameChange, name, Serialize::patchNameChangeDataSize>().size());
    constexpr PatchSettingChange change1{.iPatch=0, .field=0, .value=0};
    static_assert(Serialize::patchSettingChangeDataSize == Serialize::objToBytes<PatchSettingChange, change1, Serialize::patchSettingChangeDataSize>().size());
    constexpr PatchOpChange change2{.iPatch=0, .iOp=0, .field=0, .value=0};
    static_assert(Serialize::opParamsChangeDataSize == Serialize::objToBytes<PatchOpChange, change2, Serialize::opParamsChangeDataSize>().size());
    static_assert((sizeof(Flash::Wrapper<SerializedPatchBank>) % 4096) == 0);
}

IN_FLASH("Patches")
void init()
{
    verifyData();

    // Initialize the patch bank from serialized storage
    Patches::loadCurrentPatchBank(initialPatchData.obj);
}

Patch& getPatch(unsigned i)
{
    if (i >= numPatches) {
        i = 0;
    }
    return patchBankCurrent.patches[i];
}

IN_FLASH("Patches")
bool loadCurrentPatchBank(const auto& storage)
{
    static PatchBank patchBankNew; // must be static because stack space is limited
    if (Serialize::readObject(storage, &patchBankNew)) {
        if (!patchBankNew.isValid()) {
            dputs("loadCurrentPatchBank: ERROR: Bad patch data");
            Error::set<Error::Err::BadPatchData>();
            return false;
        }
        patchBankCurrent = patchBankNew;
        return true;
    } else {
        return false;
    }
}

IN_FLASH("Patches")
size_t saveCurrentPatchBank(auto* pstorage)
{
    return Serialize::writeObject(*pstorage, patchBankCurrent);
}

IN_FLASH("Patches")
void saveInitialPatchData(const SerializedPatchBank& patchData)
{
    Flash::copyToFlash(patchData, &initialPatchData);
}

} } // namespace Patches
