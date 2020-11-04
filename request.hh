#ifndef __MEM_REQUEST_HH__
#define __MEM_REQUEST_HH__

#include "types.hh"

class Request
{
  private:
        Addr _paddr = 0;
        unsigned _size = 0;
        Addr _vaddr = MaxAddr;

  public:
    Request(Addr vaddr, unsigned size):
        _vaddr(vaddr), _size(size)
    {

    }


    Addr
    getVaddr() const
    {
        return _vaddr;
    }


    void
    setPaddr(Addr paddr)
    {
        _paddr = paddr;
    }

    Addr
    getPaddr() const
    {
        return _paddr;
    }

    unsigned
    getSize() const
    {
        return _size;
    }
};

typedef Request* RequestPtr;



#endif // __MEM_REQUEST_HH__