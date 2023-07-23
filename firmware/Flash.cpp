namespace Dexy { namespace Flash {

template<typename T>
__attribute__((noinline))
void copyToFlash(const T& objFrom, Wrapper<T>* pobjTo)
{
    unsigned addrTo = unsigned(pobjTo);
    unsigned offsetTo = addrTo - XIP_BASE;
    constexpr unsigned cbErase = sizeof(*pobjTo);
    constexpr unsigned cbProgram = FLASH_PAGE_ADJUST(sizeof(T));
    static_assert(cbErase % FLASH_SECTOR_SIZE == 0);
    static_assert(cbProgram % FLASH_PAGE_SIZE == 0);
    dassert(offsetTo + cbProgram <= PICO_FLASH_SIZE_BYTES, BadFlashData);
    {
        Lockout lockout; // disable interrupts and stop core 1 during flash programming
        flash_range_erase(offsetTo, cbErase);
        flash_range_program(offsetTo, (const uint8_t*)&objFrom, cbProgram);
    }
}

} } // namespace Flash
