#include "faults.hh"

#include "base/logging.hh"
#include "types.hh"
#include "thread_context.hh"

#include <string>

class ThreadContext;

namespace X86ISA
{
    void
    PageFault::invoke(ThreadContext *tc){
        if (!tc->fixupFault(addr)) {
            // print information about what we are panic'ing on
            panic("error");
        }
    }

    void 
    GenericPageTableFault::invoke(ThreadContext *tc){
        printf("file:%s\n", __FILE__);
    }
} // end namespace X86ISA
