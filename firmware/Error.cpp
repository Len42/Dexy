namespace Dexy { namespace Error {

#ifndef DEBUG_CHECK_ERRORS

// Don't do anything.
template<Err err> bool isSet() { return false; }
template<Err err> void set() { }
template<Err err> void reset() { }
template<Err err> void assertion([[maybe_unused]] bool f) { }
bool anySet() { return false; }
void clearAll() { }
void dumpAllSetErrors() { }

#else // DEBUG_CHECK_ERRORS

/// @brief Info about a particular error code
class ErrorType
{
public:
    explicit constexpr ErrorType(const char* nameIn) : name(nameIn), fSet(false) { }
    bool isSet() const { return fSet; }
    void set() { fSet = true; }
    void clear() { fSet = false; }
    const char* getName() const { return name; }
private:
    const char* name;
    bool fSet;
};

static constinit ErrorType errorTable[] = {
#define DECLARE_ERRORTYPE(name, ...) ErrorType(#name),
    FOR_EACH_ERROR_TYPE(DECLARE_ERRORTYPE)
};

template<Err err>
static ErrorType& getErr()
{
    static_assert(unsigned(err) < std::size(errorTable));
    return errorTable[unsigned(err)];
}

template<Err err>
void set()
{
    getErr<err>().set();
}

template<Err err>
bool isSet()
{
    return getErr<err>().isSet();
}

template<Err err>
void clear()
{
    getErr<err>().clear();
}

template<Err err>
void assertion(bool f)
{
    if (!f) {
        getErr<err>().set();
    }
}

bool anySet()
{
    return std::ranges::any_of(errorTable, [](auto&& error){ return error.isSet(); });
}

void clearAll()
{
    for (auto&& error : errorTable) {
        error.clear();
    }
}

void dumpAllSetErrors()
{
    for (auto&& error : errorTable) {
        if (error.isSet()) {
            printf("err%s ", error.getName());
        }
    }
    putchar('\n');
}

#endif // DEBUG_CHECK_ERRORS

} } // namespace Error
