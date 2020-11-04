#include "process.hh"
#include "system.hh"


void
Process::allocateMem(Addr vaddr, int64_t size)
{
    int npages = divCeil(size, (int64_t)system->getPageBytes());
    Addr paddr = system->allocPhysPages(npages);
    pTable->map(vaddr, paddr, size);
}