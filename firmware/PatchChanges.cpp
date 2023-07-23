namespace Dexy { namespace Patches {

// Define identifiers for the data members in Patch & OpParams, for use in
// patchFields and opParamsFields.
// The order must match the indices sent by the patch editor.

/// @brief List of Patch data members
/// @see https://en.wikipedia.org/wiki/X_macro
#define FOR_EACH_PATCH_FIELD(DO) \
    DO(algorithm) \
    DO(feedbackAmount)

/// @brief List of OpParams data members
/// @see https://en.wikipedia.org/wiki/X_macro
#define FOR_EACH_OPPARAMS_FIELD(DO) \
    DO(fixedFreq) \
    DO(noteOrFreq) \
    DO(outputLevel) \
    DO(useEnvelope) \
    DO(ampModSens)

/// @brief List of EnvParams data members
/// @see https://en.wikipedia.org/wiki/X_macro
#define FOR_EACH_ENVPARAMS_FIELD(DO) \
    DO(delay) \
    DO(attack) \
    DO(decay) \
    DO(sustain) \
    DO(release) \
    DO(loop)

// Map field numbers to member pointers for Patch, OpParams, and EnvParams.

using PatchField = std::variant<bool Patch::*,
                                uint8_t Patch::*,
                                param_t Patch::*>;

/// @brief List of Patch data member locations, used by mergePatchChange()
static constexpr PatchField patchFields[] = {
    #define DEFINE_PATCHFIELD(field, ...) &Patch::field, 
    FOR_EACH_PATCH_FIELD(DEFINE_PATCHFIELD)
};

using OpParamsField = std::variant<bool OpParams::*,
                                   param_t OpParams::*,
                                   bool EnvParams::*,
                                   param_t EnvParams::*>;

/// @brief List of OpParams data member locations, used by mergePatchChange()
static constexpr OpParamsField opParamsFields[] = {
    #define DEFINE_OPPARAMSFIELD(field, ...) &OpParams::field, 
    FOR_EACH_OPPARAMS_FIELD(DEFINE_OPPARAMSFIELD)
    #define DEFINE_OPPARAMSENVFIELD(field, ...) &EnvParams::field, 
    FOR_EACH_ENVPARAMS_FIELD(DEFINE_OPPARAMSENVFIELD)
};

#ifdef DEBUG_PRINT_PATCH_UPDATES

// Print debugging info when updates are handled

static constexpr const char* patchFieldNames[] = {
    #define DEFINE_PATCHFIELD_NAME(field, ...) #field,
    FOR_EACH_PATCH_FIELD(DEFINE_PATCHFIELD_NAME)
};

static constexpr const char* opParamsFieldNames[] = {
    #define DEFINE_OPPARAMSFIELD_NAME(field, ...) #field,
    FOR_EACH_OPPARAMS_FIELD(DEFINE_OPPARAMSFIELD_NAME)
    #define DEFINE_ENVPARAMSFIELD_NAME(field, ...) "env." #field,
    FOR_EACH_ENVPARAMS_FIELD(DEFINE_ENVPARAMSFIELD_NAME)
};

#define debugMergeInfo(fmt, ...) dprintf("OK\n" fmt "\n", __VA_ARGS__)

#else

#define debugMergeInfo(fmt, ...)

#endif

/// @brief If the currently-playing Patch has been updated, re-load it from the
/// PatchBank
/// @param iPatch Patch number
IN_FLASH("Patches")
static void checkReloadPatch(unsigned iPatch)
{
    if (iPatch == Synth::getCurrentPatchNum()) {
        Synth::loadPatch(iPatch);
    }
}

IN_FLASH("Patches")
void mergePatchChange(const PatchChange& change)
{
    if (change.iPatch < numPatches) {
        debugMergeInfo("Patch %u replaced", change.iPatch);
        patchBankCurrent.patches[change.iPatch] = change.patch;
        checkReloadPatch(change.iPatch);
    } else {
        Error::set<Error::Err::BadPatchData>();
    }
}

IN_FLASH("Patches")
void mergePatchChange(const PatchNameChange& change)
{
    if (change.iPatch < numPatches) {
        debugMergeInfo("Patch %u name -> %s", change.iPatch,
            std::string(std::begin(change.name), std::end(change.name)).c_str());
        patchBankCurrent.patches[change.iPatch].name = change.name;
        checkReloadPatch(change.iPatch);
    } else {
        Error::set<Error::Err::BadPatchData>();
    }
}

IN_FLASH("Patches")
void mergePatchChange(const PatchSettingChange& change)
{
    if (change.iPatch < numPatches && change.field < std::size(patchFields)) {
        debugMergeInfo("Patch %u %s -> %u",
            change.iPatch, patchFieldNames[change.field], change.value);
        Patch& patch = patchBankCurrent.patches[change.iPatch];
        const auto& field = patchFields[change.field];
        if (auto pbool = std::get_if<bool Patch::*>(&field)) {
            patch.*(*pbool) = bool(change.value);
        } else if (auto pbyte = std::get_if<uint8_t Patch::*>(&field)) {
            patch.*(*pbyte) = uint8_t(change.value);
        } else if (auto pparam = std::get_if<param_t Patch::*>(&field)) {
            patch.*(*pparam) = param_t(change.value);
        } else {
            Error::set<Error::Err::BadPatchData>();
        }
        checkReloadPatch(change.iPatch);
    } else {
        Error::set<Error::Err::BadPatchData>();
    }
}

IN_FLASH("Patches")
void mergePatchChange(const PatchOpChange& change)
{
    if (change.iPatch < numPatches && change.iOp < numOperators
        && change.field < std::size(opParamsFields))
    {
        debugMergeInfo("Patch %u op %u %s -> %u",
            change.iPatch, change.iOp, opParamsFieldNames[change.field], change.value);
        Patch& patch = patchBankCurrent.patches[change.iPatch];
        OpParams& opParams = patch.opParams[change.iOp];
        const auto& field = opParamsFields[change.field];  
        if (auto pbool0 = std::get_if<bool OpParams::*>(&field)) {
            opParams.*(*pbool0) = bool(change.value);
        } else if (auto pparam0 = std::get_if<param_t OpParams::*>(&field)) {
            opParams.*(*pparam0) = param_t(change.value);
        } else if (auto pbool1 = std::get_if<bool EnvParams::*>(&field)) {
            opParams.env.*(*pbool1) = bool(change.value);
        } else if (auto pparam1 = std::get_if<param_t EnvParams::*>(&field)) {
            opParams.env.*(*pparam1) = param_t(change.value);
        } else {
            Error::set<Error::Err::BadPatchData>();
        }
        checkReloadPatch(change.iPatch);
    } else {
        Error::set<Error::Err::BadPatchData>();
    }
}

} } // namespace Patches
