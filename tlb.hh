/*
 * Copyright (c) 2007 The Hewlett-Packard Development Company
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __TLB_HH__
#define __TLB_HH__

#include <list>
#include <vector>
#include <string>

// #include "arch/generic/tlb.hh"
// #include "arch/x86/pagetable.hh"
// #include "base/trie.hh"
// #include "mem/request.hh"
// #include "params/X86TLB.hh"
// #include "sim/stats.hh"

#include "base/trie.hh"
#include "types.hh"
#include "request.hh"
#include "translation.hh"

#include "thread_context.hh"

class ThreadContext;
class DataTranslation;

namespace X86ISA
{
    struct TlbEntry;
    
    typedef Trie<Addr, X86ISA::TlbEntry> TlbEntryTrie;

    struct TlbEntry
    {
        // The base of the physical page.
        Addr paddr;

        // The beginning of the virtual page this entry maps.
        Addr vaddr;
        // The size of the page this represents, in address bits.
        unsigned logBytes;

        // Read permission is always available, assuming it isn't blocked by
        // other mechanisms.
        bool writable;
        // Whether this page is accesible without being in supervisor mode.
        bool user;
        // Whether to use write through or write back. M5 ignores this and
        // lets the caches handle the writeback policy.
        //bool pwt;
        // Whether the page is cacheable or not.
        bool uncacheable;
        // Whether or not to kick this page out on a write to CR3.
        bool global;
        // A bit used to form an index into the PAT table.
        bool patBit;
        // Whether or not memory on this page can be executed.
        bool noExec;
        // A sequence number to keep track of LRU.
        uint64_t lruSeq;

        TlbEntryTrie::Handle trieHandle;

        TlbEntry(Addr asn, Addr _vaddr, Addr _paddr,
                 bool uncacheable, bool read_only)
                 :vaddr(_vaddr), paddr(_paddr)
                 {

                 }
        TlbEntry(){
                
        }

        void
        updateVaddr(Addr new_vaddr)
        {
            vaddr = new_vaddr;
        }

        Addr pageStart()
        {
            return paddr;
        }

        // Return the page size in bytes
        int size()
        {
            return (1 << logBytes);
        }
    };

}

namespace X86ISA
{
    enum Mode { Read, Write, Execute };

    class TLB// : public BaseTLB
    {
      public:
      protected:
        typedef std::list<TlbEntry *> EntryList;

        uint32_t configAddress;

      public:

        struct TLBParams{
            std::string name;
            size_t size;
        };
        typedef TLBParams Params;
        TLB(Params *p);

        TlbEntry *lookup(Addr va, bool update_lru = true);

      protected:

        EntryList::iterator lookupIt(Addr va, bool update_lru = true);

      public:
        void flushAll();

        void flushNonGlobal();

        void demapPage(Addr va, uint64_t asn);

        void setConfigAddress(uint32_t addr);

      protected:
        uint32_t size;

        std::vector<TlbEntry> tlb;

        EntryList freeList;

        TlbEntryTrie trie;
        uint64_t lruSeq;

        // struct TlbStats{
        //     TlbStats();
        //     int rdAccesses;
        //     int wrAccesses;
        //     int rdMisses;
        //     int wrMisses;
        // } stats;

        PageFault* translateInt(bool read, RequestPtr req, ThreadContext *tc);
        PageFault* translate(const RequestPtr &req, ThreadContext *tc, DataTranslation *translation);

      public:

        void evictLRU();

        uint64_t
        nextSeq()
        {
            return ++lruSeq;
        }

        // PageFault*  translateAtomic(
        //     const RequestPtr &req, ThreadContext *tc, Mode mode);
        // PageFault*  translateFunctional(
        //     const RequestPtr &req, ThreadContext *tc, Mode mode);
        void translateTiming(
            const RequestPtr &req, ThreadContext *tc,
            DataTranslation *translation, Mode mode);

        /**
         * Do post-translation physical address finalization.
         *
         * Some addresses, for example requests going to the APIC,
         * need post-translation updates. Such physical addresses are
         * remapped into a "magic" part of the physical address space
         * by this method.
         *
         * @param req Request to updated in-place.
         * @param tc Thread context that created the request.
         * @param mode Request type (read/write/execute).
         * @return A fault on failure, NoFault otherwise.
         */
        // PageFault*  finalizePhysical(const RequestPtr &req, ThreadContext *tc,
        //                        Mode mode) const;

        TlbEntry *insert(Addr vpn, const TlbEntry &entry);
    };
}

#endif // __ARCH_X86_TLB_HH__
