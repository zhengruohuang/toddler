#include "common/include/data.h"
#include "hal/include/print.h"
#include "hal/include/int.h"


int_handler int_handler_list[IDT_ENTRY_COUNT];

static u32 interrupt_lock = 0;


/*
 * Disable LOCAL interrupts
 */
void disable_local_int()
{
    __asm__ __volatile__
    (
        "cli;"
        :
        :
    );
}

/*
 * Enable local interrupts
 */
void enable_local_int()
{
    __asm__ __volatile__
    (
        "sti;"
        :
        :
    );
}

/*
 * Initialize int handlers
 */
void init_int_handlers()
{
    u32 i;
    
    kprintf("Initializing interrupt handlers ... ");
    
    for (i = 0; i < IDT_ENTRY_COUNT; i++) {
        int_handler_list[i] = NULL;
    }
    
    kprintf("Done!\n");
}

/*
 * General interrupt handler entry
 */
int asmlinkage int_handler_entry(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
{
    // For debugging, only allow one int handler at a time in a mp system
//     do {
//         if (hal_atomic_cas(&interrupt_lock, 0, 1)) {
//             break;
//         }
//     } while (1);
    
    kprintf("VecNum %h, ErrCode %h, Eip %h, Cs %d, Eflags %b\n", vector_num, error_code, eip, cs, eflags);
    
    //kprintf("Current Processor Index %d, ID %h, Area at %h\n",
    //hal_sysinfo_get_local()->processor_number,
    //hal_sysinfo_get_local()->processor_id,
    //hal_sysinfo_get_local()->processor_area_address
    //);
    
    /*
     * A flag indicates whether we should invoke hal worker after this interrupt
     */
    u32 invoke_hal_worker = 1;
    
    /* Obtain handler */
    int_handler handler = int_handler_list[vector_num];
    if (NULL == (void *)handler) {
        handler = int_handler_dummy;
    }
    
    interrupt_lock = 0;
    
    /*
     * Call the handler
     *
     * Note that interrupts may be enabled by the handler to support
     * nested interrupting, so we need to diable local interrupts after
     * this function no matter whether the handler would disable interrupts
     * itself
     */
    invoke_hal_worker = handler(vector_num, error_code, eip, cs, eflags);
    
    /*
     * After this function call, no other interrupts would occur,
     * thus nested interrupting would be disabled
     */
    disable_local_int();
    
    /* If we need to invoke the hal worker, current context may already been switched to ASMgr's by message creation */
    if (invoke_hal_worker) {
        //hal_ipc_create_message();
        
        /* Invoker HAL worker */
        //hal_task_invoke_hal_worker();
        //kprintf(">>> Here!\n");
    }
    
    //kprintf("To return\n");
    
    //s_context_info *cur_context = *((s_context_info **)hal_per_processor_variable_get_current(hal_switch_current_context_ptr_per_cpu_offset));
    //unsigned long return_value = cur_context ? cur_context->page_table_pfn << 12 : 0;
    
    return 0;
}

/*
 * Default dummy handler: don't do anything
 */
int int_handler_dummy(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
{
    //kprintf("VecNum %h, ErrCode %h, Eip %h, Cs %d, Eflags %b\n", vector_num, error_code, eip, cs, eflags);
//     kprintf("Current Processor Index %d, ID %h, Area at %h\n",
//                hal_sysinfo_get_local()->processor_number,
//                hal_sysinfo_get_local()->processor_id,
//                hal_sysinfo_get_local()->processor_area_address
//     );
//     
//     if (0x33 == vector_num) {
//         BREAK("");
//     }
//     
//     kprintf("!!! Interrupt Occured (VecNum %d, ErrCode %h) !!!\n", vector_num, error_code);
//     
//     u32 cr2;
//     
//     __asm__ __volatile__ (
//         "movl   %%cr2, %%eax;"
//         : "=a" (cr2)
//         :
//     );
//     
//     kprintf("CR2 %h\n", cr2);
//     
//     //hal_apic_lapic_eoi();
//     BREAK("");
    
    return 0;
}

/*
 * Exception
 */
int int_handler_exception(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
{
    kprintf("\n!!! Exception Occured (VecNum %d, ErrCode %h) !!!\n", vector_num, error_code);
    
    return 1;
}

/*
 * Device
 */
int int_handler_device(u32 vector_num, u32 error_code, u32 eip, u32 cs, u32 eflags)
{
    kprintf("\n!!! Device (VecNum %d, IrqNum %h) !!!\n", vector_num, vector_num - 32);
    
    return 1;
}
/******************************************************************************/
