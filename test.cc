#include "tlb.hh"
#include "types.hh"
#include "system.hh"
#include "thread_context.hh"
// #include "mmu.hh"
#include "process.hh"

class Tlb_Api{
  public:
    System* sys;
    X86ISA::TLB *tlb;
    

    // 假定最多有5个进程
    const int process_num = 5;
    Process *process_array[5];
    int curProcess = 0;

    void
    init(size_t hbm_size){
        // init memory System
        // size_t hbm_size = 1*1024*1024;
        sys = new System(hbm_size);

        for (int i=0; i<process_num; i++){
            process_array[i] = nullptr;
        }

        // init tlb
        X86ISA::TLB::Params* tlb_params = new X86ISA::TLB::Params();
        tlb_params->name = "tlb";
        tlb_params->size = 128;
        tlb = new X86ISA::TLB(tlb_params);
    }

    void
    newProcess(uint64_t pid){
        // page table
        // uint64_t pid = 1;
        Addr pageSize = sys->getPageBytes();
        EmulationPageTable* pTable = new EmulationPageTable("ptable", pid, pageSize);
        // pid
        Process::ProcessParams *process_params = new Process::ProcessParams();
        process_params->pid = pid;
        // create
        for (int i=0; i<process_num; i++){
            if(!process_array[i]){
                curProcess = i;
                process_array[i] = new Process(process_params, pTable, sys);
            }
        }
    }

    void freeProcess(uint64_t pid){
        ;
    }

    void switchProcess(uint64_t pid){
        for (int i=0; i<process_num; i++){
            if(process_array[i] && process_array[i]->pid==pid){
                curProcess = i;
            }
        }
        tlb->flushAll();
    }

    void
    translate(Addr vaddr, size_t size){
        ThreadContext *tc = new ThreadContext(process_array[curProcess]);
        // request
        DataTranslation *trans = new DataTranslation();
        RequestPtr req = new Request(vaddr, size);
        tlb->translateTiming(req, tc, trans, X86ISA::Read);
        // tlb->translateTiming(new 
    }
};

int main()
{
    Tlb_Api api;
    api.init(1*1024*1024);
    api.newProcess(1);
    api.translate(1*4096, 64);
    api.translate(2*4096, 64);
    api.translate(3*4096, 64);
    api.translate(3*4096, 64);
    api.translate(3*4096, 64);
    api.translate(1*4096, 64);
    api.translate(1*4096, 64);
    api.translate(4*4096, 64);
    api.translate(4*4096, 64);
}