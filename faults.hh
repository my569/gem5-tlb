#ifndef __ARCH_X86_FAULTS_HH__
#define __ARCH_X86_FAULTS_HH__

#include "base/logging.hh"
#include "types.hh"
// #include "thread_context.hh"

#include <string>

class ThreadContext;

namespace X86ISA
{
    class PageFault
    {
    protected:
        Addr addr;

    public:
        PageFault(Addr _addr, uint32_t _errorCode) :
            addr(_addr)
        {}

        void
        invoke(ThreadContext *tc);
    };

    class GenericPageTableFault
    {
    private:
        Addr vaddr;
    public:
        std::string name() const { return "Generic page table fault"; }
        GenericPageTableFault(Addr va) : vaddr(va) {}
        void invoke(ThreadContext *tc);
        Addr getFaultVAddr() const { return vaddr; }
    };

} // end namespace X86ISA


#endif // __ARCH_X86_FAULTS_HH__
