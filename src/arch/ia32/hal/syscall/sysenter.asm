[section .text]

;===============================================================================
; Imports
;===============================================================================
extern save_context_sysenter
extern sysenter_handler_entry

extern syscall_proxy_entry
;===============================================================================


;===============================================================================
;  Exports
;===============================================================================
global sysenter_handler

global sysenter_proxy_start_origin
global sysenter_proxy_end_origin
;===============================================================================


;===============================================================================
; Syscall
;===============================================================================
;-------------------------------------------------------------------------------
; Syscall Handler Entry
;-------------------------------------------------------------------------------
sysenter_handler:
    ; Bring the IF flag to a known state
    cli

    ; Save the task status
    sub     esp, 4 * 7      ; SS, ESP, EIP, CS, EFLAGS, ERROR CODE, VEC NUM

    pushad

    xor     eax, eax
    mov     ax, ds
    push    eax
    mov     ax, es
    push    eax
    mov     ax, fs
    push    eax
    mov     ax, gs
    push    eax

    ; Save task status and handle this syscall
    mov     eax, esp
    push    eax
    call    save_context_sysenter
    call    sysenter_handler_entry
    add     esp, 4

    pop     eax
    mov     gs, ax
    pop     eax
    mov     fs, ax
    pop     eax
    mov     es, ax
    pop     eax
    mov     ds, ax
    popad
    
    sti
    sysexit

    ; Should not arrive here
    jmp     $
    
;-------------------------------------------------------------------------------
; Perform Fast System Call in User Mode
;-------------------------------------------------------------------------------
;  Usage: Move the parameters to eax, ebx, esi, edi, and ecx or edx should not
;         be used, and then do "call hal_syscall_perform" to perform a syscall.
;-------------------------------------------------------------------------------
ALIGN   16
sysenter_proxy_start_origin:
    mov     ecx, esp
    mov     edx, [syscall_proxy_entry]
    add     edx, .sysenter_return_point - sysenter_proxy_start_origin
    
    sysenter
    
ALIGN   16
.sysenter_return_point:
    ret
    
ALIGN   16
sysenter_proxy_end_origin:
;-------------------------------------------------------------------------------
;===============================================================================
