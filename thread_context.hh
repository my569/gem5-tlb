#ifndef __CPU_THREAD_CONTEXT_HH__
#define __CPU_THREAD_CONTEXT_HH__

#include <iostream>
#include <string>

#include "base/logging.hh"
#include "types.hh"
#include "page_table.hh"
#include "system.hh"
#include "process.hh"
// #include "faults.hh"

class ThreadContext
{
  public:
    Process* p;

    ThreadContext(Process* process):
        p(process)
    {

    }

    bool fixupFault(Addr vaddr){
        return p->MemState_fixupFault(vaddr);
    }

    Process*
    getProcessPtr(){
        return p;
    }

};


#endif
