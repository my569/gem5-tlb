
#include "faults.hh"
#include "types.hh"
#include "request.hh"
#include "thread_context.hh"

class CPU{
    ThreadContext *tc;
    void
    finishTranslation(X86ISA::PageFault* fault,
                                        const RequestPtr &mem_req)
    {
        fault->invoke(tc);
    }
};
