
#ifndef __SYSTEM_HH__
#define __SYSTEM_HH__

#include "types.hh"
#include "base/logging.hh"

class System
{
  public:
    const Addr PageShift = 12;
    const Addr PageBytes = ULL(1) << PageShift;

    Addr pagePtr;
    Addr totalSize;//

    System(Addr _totalSize)
    : pagePtr(0), totalSize(_totalSize){
        
    }

    Addr getPageBytes() const { return PageBytes; }


    Addr
    allocPhysPages(int npages)
    {
        Addr return_addr = pagePtr << PageShift;
        pagePtr += npages;

        Addr next_return_addr = pagePtr << PageShift;

        if ((pagePtr << PageShift) > totalSize){
            assert((pagePtr << PageShift) < totalSize);
            fatal("Out of memory, please increase size of physical memory.");
        }
        return return_addr;
    }

};

#endif // __SYSTEM_HH__
