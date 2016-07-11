#include "common/include/data.h"
#include "common/include/atomic.h"
#include "kernel/include/hal.h"
#include "kernel/include/mem.h"
#include "kernel/include/proc.h"


struct tlb_shootdown_record {
    int valid;
    
    ulong addr;
    size_t size;
    
    volatile ulong response_count;
    volatile char *response_records;
};


static volatile struct tlb_shootdown_record *records;


void init_tlb_mgmt()
{
    int i, j;
    
    records = malloc(sizeof(struct tlb_shootdown_record) * hal->num_cpus);
    for (i = 0; i < hal->num_cpus; i++) {
        records[i].valid = 0;
        records[i].response_count = 0;
        
        records[i].response_records = malloc(sizeof(char) * hal->num_cpus);
        for (j = 0; j < hal->num_cpus; j++) {
            records[i].response_records[j] = 0;
        }
    }
    
    membar();
}

void trigger_tlb_shootdown(ulong addr, size_t size)
{
    int i;
    int cpu_count = hal->num_cpus;
    int cur_cpu_id = hal->get_cur_cpu_id();
    
    kprintf("[TLB] cpu count: %d\n", cpu_count);
    
    assert(!records[cur_cpu_id].valid);
    hal->invalidate_tlb(0, addr, size);
    
    records[cur_cpu_id].addr = addr;
    records[cur_cpu_id].size = size;
    records[cur_cpu_id].response_count = 1;
    for (i = 0; i < cpu_count; i++) {
        records[cur_cpu_id].response_records[i] = 0;
    }
    records[cur_cpu_id].response_records[cur_cpu_id] = 1;
    
    membar();
    records[cur_cpu_id].valid = 1;
    
    kprintf("[TLB] TLB shootdown triggered, addr: %u, size: %u\n", addr, size);
    
    while (records[cur_cpu_id].response_count < cpu_count) {
        hal->yield();
    }
    
    membar();
    
    records[cur_cpu_id].valid = 0;
    membar();
    
    kprintf("[TLB] TLB shootdown done, addr: %u, size: %u\n", addr, size);
}


void service_tlb_shootdown()
{
    int i;
    int cur_cpu_id = hal->get_cur_cpu_id();
    
    for (i = 0; i < hal->num_cpus; i++) {
        if (!records[i].valid) {
            continue;
        }
        
        if (records[i].response_records[cur_cpu_id]) {
            continue;
        }
        
        hal->invalidate_tlb(0, records[i].addr, records[i].size);
        records[i].response_records[cur_cpu_id] = 1;
        membar();
        atomic_inc(&records[i].response_count);
        
        kprintf("[TLB] TLB shootdown serviced!\n");
    }
}
