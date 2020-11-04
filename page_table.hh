/**
 * @file
 * Declarations of a non-full system Page Table.
 */

#ifndef __MEM_PAGE_TABLE_HH__
#define __MEM_PAGE_TABLE_HH__

#include <string>
#include <vector>
#include <unordered_map>

#include "types.hh"
#include "base/intmath.hh"
#include "base/bitfield.hh"
#include "request.hh"

// #include "thread_context.hh"
#include "faults.hh"

class ThreadContext;

class EmulationPageTable 
{
  public:
    struct Entry
    {
        Addr paddr;
        uint64_t flags;

        Entry(Addr paddr, uint64_t flags) : paddr(paddr), flags(flags) {}
        Entry() {}
    };

  protected:
    typedef std::unordered_map<Addr, Entry> PTable;
    typedef PTable::iterator PTableItr;
    PTable pTable;

    const Addr pageSize;
    const Addr offsetMask;

    const uint64_t _pid;
    const std::string _name;

  public:

    EmulationPageTable(
            const std::string &__name, uint64_t _pid, Addr _pageSize) :
            pageSize(_pageSize), offsetMask(mask(floorLog2(_pageSize))),
            _pid(_pid), _name(__name), shared(false)
    {
        assert(isPowerOf2(pageSize));
    }

    uint64_t pid() const { return _pid; };

    virtual ~EmulationPageTable() {};

    /* generic page table mapping flags
     *              unset | set
     * bit 0 - no-clobber | clobber
     * bit 2 - cacheable  | uncacheable
     * bit 3 - read-write | read-only
     */
    enum MappingFlags : uint32_t {
        Clobber     = 1,
        Uncacheable = 4,
        ReadOnly    = 8,
    };

    // flag which marks the page table as shared among software threads
    bool shared;

    virtual void initState() {};

    // for DPRINTF compatibility
    const std::string name() const { return _name; }

    Addr pageAlign(Addr a)  { return (a & ~offsetMask); }
    Addr pageOffset(Addr a) { return (a &  offsetMask); }

    /**
     * Maps a virtual memory region to a physical memory region.
     * @param vaddr The starting virtual address of the region.
     * @param paddr The starting physical address where the region is mapped.
     * @param size The length of the region.
     * @param flags Generic mapping flags that can be set by or-ing values
     *              from MappingFlags enum.
     */
    virtual void map(Addr vaddr, Addr paddr, int64_t size, uint64_t flags = 0);
    virtual void remap(Addr vaddr, int64_t size, Addr new_vaddr);
    virtual void unmap(Addr vaddr, int64_t size);

    /**
     * Check if any pages in a region are already allocated
     * @param vaddr The starting virtual address of the region.
     * @param size The length of the region.
     * @return True if no pages in the region are mapped.
     */
    virtual bool isUnmapped(Addr vaddr, int64_t size);

    /**
     * Lookup function
     * @param vaddr The virtual address.
     * @return The page table entry corresponding to vaddr.
     */
    const Entry *lookup(Addr vaddr);

    /**
     * Translate function
     * @param vaddr The virtual address.
     * @param paddr Physical address from translation.
     * @return True if translation exists
     */
    bool translate(Addr vaddr, Addr &paddr);

    /**
     * Simplified translate function (just check for translation)
     * @param vaddr The virtual address.
     * @return True if translation exists
     */
    bool translate(Addr vaddr) { Addr dummy; return translate(vaddr, dummy); }

    /**
     * Perform a translation on the memory request, fills in paddr
     * field of req.
     * @param req The memory request.
     */
    //Fault translate(const RequestPtr &req);
    X86ISA::GenericPageTableFault* translate(const RequestPtr &req);

    void getMappings(std::vector<std::pair<Addr, Addr>> *addr_mappings);
};

#endif // __MEM_PAGE_TABLE_HH__
