#ifndef __PROCESS_HH__
#define __PROCESS_HH__

#include "page_table.hh"
#include "system.hh"

class Process
{
  public:
    EmulationPageTable* pTable;
    System *system;
    const int64_t _pageBytes;
    uint64_t pid;

    struct ProcessParams{
        uint64_t pid;
    };

    Process(ProcessParams *params, EmulationPageTable* _pTable, System* _system):
        pTable(_pTable), system(_system),
        _pageBytes(system->getPageBytes()),
        pid(params->pid)
    {
    }
    // void
    // CPU_finishTranslation(ThreadContext *tc, X86ISA::PageFault* fault){
    //     fault->invoke(tc);
    // }

    bool
    MemState_fixupFault(Addr vaddr)
    {
        //Addr paddr = 
        allocateMem(vaddr, _pageBytes);
        // pTable->map(vaddr, paddr, _pageBytes*4096);
        return true;
    }

    void
    allocateMem(Addr vaddr, int64_t size);
};


#endif