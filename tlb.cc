/*
 * Copyright (c) 2007-2008 The Hewlett-Packard Development Company
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

// #include "arch/x86/tlb.hh"

#include <cstring>
#include <memory>

#include "tlb.hh"
#include "faults.hh"
#include "base/logging.hh"
#include "translation.hh"
#include "process.hh"

// #include "arch/x86/faults.hh"
// #include "arch/x86/insts/microldstop.hh"
// #include "arch/x86/pagetable_walker.hh"
// #include "arch/x86/pseudo_inst_abi.hh"
// #include "arch/x86/regs/misc.hh"
// #include "arch/x86/regs/msr.hh"
// #include "arch/x86/x86_traits.hh"
// #include "base/trace.hh"
// #include "cpu/thread_context.hh"
// #include "debug/TLB.hh"
// #include "mem/packet_access.hh"
// #include "mem/page_table.hh"
// #include "mem/request.hh"
// #include "sim/full_system.hh"
// #include "sim/process.hh"
// #include "sim/pseudo_inst.hh"

#include "tlb.hh"

namespace X86ISA {

TLB::TLB(X86ISA::TLB::Params* p)
    : 
    // BaseTLB(p), 
    configAddress(0), 
    size(p->size),
      tlb(size), 
      lruSeq(0)
{
    if (!size)
        fatal("TLBs must have a non-zero size.\n");

    for (int x = 0; x < size; x++) {
        tlb[x].trieHandle = NULL;
        freeList.push_back(&tlb[x]);
    }
}

void
TLB::evictLRU()
{
    // Find the entry with the lowest (and hence least recently updated)
    // sequence number.

    unsigned lru = 0;
    for (unsigned i = 1; i < size; i++) {
        if (tlb[i].lruSeq < tlb[lru].lruSeq)
            lru = i;
    }

    assert(tlb[lru].trieHandle);
    trie.remove(tlb[lru].trieHandle);
    tlb[lru].trieHandle = NULL;
    freeList.push_back(&tlb[lru]);
}

TlbEntry *
TLB::insert(Addr vpn, const TlbEntry &entry)
{
    // If somebody beat us to it, just use that existing entry.
    TlbEntry *newEntry = trie.lookup(vpn);
    if (newEntry) {
        assert(newEntry->vaddr == vpn);
        return newEntry;
    }

    if (freeList.empty())
        evictLRU();

    newEntry = freeList.front();
    freeList.pop_front();

    *newEntry = entry;
    newEntry->lruSeq = nextSeq();
    newEntry->vaddr = vpn;
    newEntry->trieHandle =
    trie.insert(vpn, TlbEntryTrie::MaxBits - entry.logBytes, newEntry);
    return newEntry;
}

TlbEntry *
TLB::lookup(Addr va, bool update_lru)
{
    TlbEntry *entry = trie.lookup(va);
    if (entry && update_lru)
        entry->lruSeq = nextSeq();
    return entry;
}

void
TLB::flushAll()
{
    DPRINTF(TLB, "Invalidating all entries.\n");
    for (unsigned i = 0; i < size; i++) {
        if (tlb[i].trieHandle) {
            trie.remove(tlb[i].trieHandle);
            tlb[i].trieHandle = NULL;
            freeList.push_back(&tlb[i]);
        }
    }
}

void
TLB::setConfigAddress(uint32_t addr)
{
    configAddress = addr;
}

void
TLB::flushNonGlobal()
{
    DPRINTF(TLB, "Invalidating all non global entries.\n");
    for (unsigned i = 0; i < size; i++) {
        if (tlb[i].trieHandle && !tlb[i].global) {
            trie.remove(tlb[i].trieHandle);
            tlb[i].trieHandle = NULL;
            freeList.push_back(&tlb[i]);
        }
    }
}

void
TLB::demapPage(Addr va, uint64_t asn)
{
    TlbEntry *entry = trie.lookup(va);
    if (entry) {
        trie.remove(entry->trieHandle);
        entry->trieHandle = NULL;
        freeList.push_back(entry);
    }
}

// namespace
// {

// Cycles
// localMiscRegAccess(bool read, MiscRegIndex regNum,
//                    ThreadContext *tc, PacketPtr pkt)
// {
//     if (read) {
//         RegVal data = htole(tc->readMiscReg(regNum));
//         assert(pkt->getSize() <= sizeof(RegVal));
//         pkt->setData((uint8_t *)&data);
//     } else {
//         RegVal data = htole(tc->readMiscRegNoEffect(regNum));
//         assert(pkt->getSize() <= sizeof(RegVal));
//         pkt->writeData((uint8_t *)&data);
//         tc->setMiscReg(regNum, letoh(data));
//     }
//     return Cycles(1);
// }

// } // anonymous namespace

PageFault* 
TLB::translateInt(bool read, RequestPtr req, ThreadContext *tc)
{
    return 0;
    // DPRINTF(TLB, "Addresses references internal memory.\n");
    // Addr vaddr = req->getVaddr();
    // Addr prefix = (vaddr >> 3) & IntAddrPrefixMask;
    // if (prefix == IntAddrPrefixCPUID) {
    //     panic("CPUID memory space not yet implemented!\n");
    // } else if (prefix == IntAddrPrefixMSR) {
    //     vaddr = (vaddr >> 3) & ~IntAddrPrefixMask;

    //     MiscRegIndex regNum;
    //     if (!msrAddrToIndex(regNum, vaddr))
    //         return std::make_shared<GeneralProtection>(0);

    //     req->setPaddr(req->getVaddr());
    //     req->setLocalAccessor(
    //         [read,regNum](ThreadContext *tc, PacketPtr pkt)
    //         {
    //             return localMiscRegAccess(read, regNum, tc, pkt);
    //         }
    //     );

    //     return NoFault;
    // } else if (prefix == IntAddrPrefixIO) {
    //     // TODO If CPL > IOPL or in virtual mode, check the I/O permission
    //     // bitmap in the TSS.

    //     Addr IOPort = vaddr & ~IntAddrPrefixMask;
    //     // Make sure the address fits in the expected 16 bit IO address
    //     // space.
    //     assert(!(IOPort & ~0xFFFF));
    //     if (IOPort == 0xCF8 && req->getSize() == 4) {
    //         req->setPaddr(req->getVaddr());
    //         req->setLocalAccessor(
    //             [read](ThreadContext *tc, PacketPtr pkt)
    //             {
    //                 return localMiscRegAccess(
    //                         read, MISCREG_PCI_CONFIG_ADDRESS, tc, pkt);
    //             }
    //         );
    //     } else if ((IOPort & ~mask(2)) == 0xCFC) {
    //         req->setFlags(Request::UNCACHEABLE | Request::STRICT_ORDER);
    //         Addr configAddress =
    //             tc->readMiscRegNoEffect(MISCREG_PCI_CONFIG_ADDRESS);
    //         if (bits(configAddress, 31, 31)) {
    //             req->setPaddr(PhysAddrPrefixPciConfig |
    //                     mbits(configAddress, 30, 2) |
    //                     (IOPort & mask(2)));
    //         } else {
    //             req->setPaddr(PhysAddrPrefixIO | IOPort);
    //         }
    //     } else {
    //         req->setFlags(Request::UNCACHEABLE | Request::STRICT_ORDER);
    //         req->setPaddr(PhysAddrPrefixIO | IOPort);
    //     }
    //     return NoFault;
    // } else {
    //     panic("Access to unrecognized internal address space %#x.\n",
    //             prefix);
    // }
}

// PageFault* 
// TLB::finalizePhysical(const RequestPtr &req,
//                       ThreadContext *tc, Mode mode) const
// {
//     // Addr paddr = req->getPaddr();

//     // if (m5opRange.contains(paddr)) {
//     //     req->setFlags(Request::STRICT_ORDER);
//     //     uint8_t func;
//     //     PseudoInst::decodeAddrOffset(paddr - m5opRange.start(), func);
//     //     req->setLocalAccessor(
//     //         [func, mode](ThreadContext *tc, PacketPtr pkt) -> Cycles
//     //         {
//     //             uint64_t ret;
//     //             PseudoInst::pseudoInst<X86PseudoInstABI, true>(tc, func, ret);
//     //             if (mode == Read)
//     //                 pkt->setLE(ret);
//     //             return Cycles(1);
//     //         }
//     //     );
//     // } else if (FullSystem) {
//     //     // Check for an access to the local APIC
//     //     LocalApicBase localApicBase =
//     //         tc->readMiscRegNoEffect(MISCREG_APIC_BASE);
//     //     AddrRange apicRange(localApicBase.base * PageBytes,
//     //                         (localApicBase.base + 1) * PageBytes);

//     //     if (apicRange.contains(paddr)) {
//     //         // The Intel developer's manuals say the below restrictions apply,
//     //         // but the linux kernel, because of a compiler optimization, breaks
//     //         // them.
//     //         /*
//     //         // Check alignment
//     //         if (paddr & ((32/8) - 1))
//     //             return new GeneralProtection(0);
//     //         // Check access size
//     //         if (req->getSize() != (32/8))
//     //             return new GeneralProtection(0);
//     //         */
//     //         // Force the access to be uncacheable.
//     //         req->setFlags(Request::UNCACHEABLE | Request::STRICT_ORDER);
//     //         req->setPaddr(x86LocalAPICAddress(tc->contextId(),
//     //                                           paddr - apicRange.start()));
//     //     }
//     // }

//     return NoFault;
// }

PageFault* 
TLB::translate(const RequestPtr &req,
        ThreadContext *tc, DataTranslation *translation)
{
    printf("%s\n", __func__);
    Addr vaddr = req->getVaddr();
    DPRINTF(TLB, "Translating vaddr %#x.\n", vaddr);

    DPRINTF(TLB, "Paging enabled.\n");
    // The vaddr already has the segment base applied.
    TlbEntry *entry = lookup(vaddr);
    // if (mode == Read) {
    //     stats.rdAccesses++;
    // } else {
    //     stats.wrAccesses++;
    // }
    if (!entry) {
        DPRINTF(TLB, "Handling a TLB miss for "
                "address %#x at pc %#x.\n",
                vaddr, tc->instAddr());
        // if (mode == Read) {
        //     stats.rdMisses++;
        // } else {
        //     stats.wrMisses++;
        // }
        // if (FullSystem) {
        //     Fault fault = walker->start(tc, translation, req, mode);
        //     if (timing || fault != NoFault) {
        //         // This gets ignored in atomic mode.
        //         delayedResponse = true;
        //         return fault;
        //     }
        //     entry = lookup(vaddr);
        //     assert(entry);
        // } else {
            //Process *p = tc->getProcessPtr();
            Process* p = tc->getProcessPtr();
            const EmulationPageTable::Entry *pte =
                p->pTable->lookup(vaddr);
            if (!pte) {
                return new PageFault(vaddr, 0);
            } else {
                Addr alignedVaddr = p->pTable->pageAlign(vaddr);
                DPRINTF(TLB, "Mapping %#x to %#x\n", alignedVaddr,
                        pte->paddr);
                entry = insert(alignedVaddr, TlbEntry(
                        p->pTable->pid(), alignedVaddr, pte->paddr,
                        pte->flags & EmulationPageTable::Uncacheable,
                        pte->flags & EmulationPageTable::ReadOnly));
            }
            DPRINTF(TLB, "Miss was serviced.\n");
        // }
    }

    DPRINTF(TLB, "Entry found with paddr %#x, "
            "doing protection checks.\n", entry->paddr);
    // Do paging protection checks.
    // bool inUser = (m5Reg.cpl == 3 &&
    //         !(flags & (CPL0FlagBit << FlagShift)));
    // CR0 cr0 = tc->readMiscRegNoEffect(MISCREG_CR0);
    // bool badWrite = (!entry->writable && (inUser || cr0.wp));
    // if ((inUser && !entry->user) || (mode == Write && badWrite)) {
    //     // The page must have been present to get into the TLB in
    //     // the first place. We'll assume the reserved bits are
    //     // fine even though we're not checking them.
    //     return std::make_shared<PageFault>(vaddr, true, mode, inUser,
    //                                        false);
    // }
    // if (storeCheck && badWrite) {
    //     // This would fault if this were a write, so return a page
    //     // fault that reflects that happening.
    //     return std::make_shared<PageFault>(vaddr, true, Write, inUser,
    //                                        false);
    // }

    Addr paddr = entry->paddr | (vaddr & mask(entry->logBytes));
    DPRINTF(TLB, "Translated %#x -> %#x.\n", vaddr, paddr);
    printf("Translated 0x%lx -> 0x%lx.\n", vaddr, paddr);
    req->setPaddr(paddr);
    // if (entry->uncacheable)
    //     req->setFlags(Request::UNCACHEABLE | Request::STRICT_ORDER);
    
    return NoFault;
}

// Fault
// TLB::translateAtomic(const RequestPtr &req, ThreadContext *tc, Mode mode)
// {
//     bool delayedResponse;
//     return TLB::translate(req, tc, NULL, mode, delayedResponse, false);
// }

// Fault
// TLB::translateFunctional(const RequestPtr &req, ThreadContext *tc, Mode mode)
// {
//     unsigned logBytes;
//     const Addr vaddr = req->getVaddr();
//     Addr addr = vaddr;
//     Addr paddr = 0;
//     if (FullSystem) {
//         Fault fault = walker->startFunctional(tc, addr, logBytes, mode);
//         if (fault != NoFault)
//             return fault;
//         paddr = insertBits(addr, logBytes - 1, 0, vaddr);
//     } else {
//         Process *process = tc->getProcessPtr();
//         const auto *pte = process->pTable->lookup(vaddr);

//         if (!pte && mode != Execute) {
//             // Check if we just need to grow the stack.
//             if (process->fixupFault(vaddr)) {
//                 // If we did, lookup the entry for the new page.
//                 pte = process->pTable->lookup(vaddr);
//             }
//         }

//         if (!pte)
//             return std::make_shared<PageFault>(vaddr, true, mode, true, false);

//         paddr = pte->paddr | process->pTable->pageOffset(vaddr);
//     }
//     DPRINTF(TLB, "Translated (functional) %#x -> %#x.\n", vaddr, paddr);
//     req->setPaddr(paddr);
//     return NoFault;
// }

void
TLB::translateTiming(const RequestPtr &req, ThreadContext *tc,
        DataTranslation *translation, Mode mode)
{
    printf("%s\n", __func__);
    // bool delayedResponse;
    assert(translation);
    PageFault* fault =
        TLB::translate(req, tc, translation);

    assert(1);//translate如果没异常，应该返回no fault
    // if (!delayedResponse)
        translation->finish(fault, req, tc);
    // else
        // translation->markDelayed();
}

} // namespace X86ISA