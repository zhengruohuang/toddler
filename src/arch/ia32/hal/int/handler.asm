[section .text]

;===============================================================================
; Imports
;===============================================================================
extern  save_context
extern  int_handler_entry
;===============================================================================


;===============================================================================
; Exports
;===============================================================================
;-------------------------------------------------------------------------------
; Predefined exception handlers
;-------------------------------------------------------------------------------
global  int_handler_divide
global  int_handler_debug
global  int_handler_nmi
global  int_handler_breakpoint
global  int_handler_overflow
global  int_handler_bounds_check
global  int_handler_invalid_opcode
global  int_handler_copr_not_available
global  int_handler_double_fault
global  int_handler_copr_seg_overrun
global  int_handler_invalid_tss
global  int_handler_segment_not_present
global  int_handler_stack_exception
global  int_handler_general_protection
global  int_handler_page_fault
global  int_handler_copr_error
global  int_handler_copr_error
global  int_handler_align_check
global  int_handler_machine_check
global  int_handler_simd_error
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; General purpose handlers
;-------------------------------------------------------------------------------
global  int_handler_general
global  int_handler_template_begin
global  int_handler_template_end
global  int_handlers
;-------------------------------------------------------------------------------
;===============================================================================


;===============================================================================
; General Handler
;===============================================================================
int_handler_general:
    ; Save all registers
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
    
    ; Save context
    mov     ebx, esp
    push    ebx
    call    save_context
    
    ; Prepare to call the handler
    add     ebx, 32 + 16
    push    dword [ebx + 4] ; Arg2:
    push    dword [ebx]     ; Arg1: 

    ; Handle the interrupt
    call    int_handler_entry

    ; Return from the interrupt
    add     esp, 8
    pop     ebx
    mov     esp, ebx

    pop     eax
    mov     gs, ax
    pop     eax
    mov     fs, ax
    pop     eax
    mov     es, ax
    pop     eax
    mov     ds, ax
    popad

    add     esp, 8

    iretd

    ; Should never arrive here
    jmp     $
;===============================================================================


;===============================================================================
; Predefined exception & trap handlers (vector = 0 - 19)
;===============================================================================
int_handler_divide:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 0                     ; vector_no = 0
    jmp     int_handler_general

int_handler_debug:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 1                     ; vector_no = 1
    jmp     int_handler_general

int_handler_nmi:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 2                     ; vector_no = 2
    jmp     int_handler_general

int_handler_breakpoint:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 3                     ; vector_no = 3
    jmp     int_handler_general

int_handler_overflow:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 4                     ; vector_no = 4
    jmp     int_handler_general

int_handler_bounds_check:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 5                     ; vector_no = 5
    jmp     int_handler_general

int_handler_invalid_opcode:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 6                     ; vector_no = 6
    jmp     int_handler_general

int_handler_copr_not_available:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 7                     ; vector_no = 7
    jmp     int_handler_general

; Note that though it is said by Intel's manual that this error has an
; error code (always zero), Qemu however does not push it automatically,
; so I guess that the handler has to push a zero as error code manually
int_handler_double_fault:
    cli
    push    dword 0
    push    dword 8                     ; vector_no = 8
    jmp     int_handler_general

int_handler_copr_seg_overrun:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 9                     ; vector_no = 9
    jmp     int_handler_general

int_handler_invalid_tss:
    cli
    push    dword 10                    ; vector_no = 10
    jmp     int_handler_general

int_handler_segment_not_present:
    cli
    push    dword 11                    ; vector_no = 11
    jmp     int_handler_general

int_handler_stack_exception:
    cli
    push    dword 12                    ; vector_no = 12
    jmp     int_handler_general

int_handler_general_protection:
    cli
    push    dword 13                    ; vector_no = 13
    jmp     int_handler_general

int_handler_page_fault:
    cli
    push    dword 14                    ; vector_no = 14
    jmp     int_handler_general

; Exception #15 is reserved by Intel

int_handler_copr_error:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 16                    ; vector_no = 16
    jmp     int_handler_general

int_handler_align_check:
    cli
    push    dword 17                    ; vector_no = 17
    jmp     int_handler_general

int_handler_machine_check:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 18                    ; vector_no = 18
    jmp     int_handler_general

int_handler_simd_error:
    cli
    push    dword 0ffffffffh            ; no err code
    push    dword 19                    ; vector_no = 19
    jmp     int_handler_general

; Exception #20 to #31 are reserved by Intel
;===============================================================================


;===============================================================================
; Interrupt handlers (vector = 32 - 255)
;===============================================================================
;-------------------------------------------------------------------------------
; General purpose handler template
;-------------------------------------------------------------------------------
ALIGN   16
int_handler_template_begin:
    cli
    push    dword 0ffffffffh            ; Dummy error code
    push    dword 0feba7632h            ; Vector Number
    jmp     dword int_handler_general

ALIGN   16
int_handler_template_end:
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; General purpose handlers
;-------------------------------------------------------------------------------
ALIGN   16

int_handlers:
%rep 256 - 32
    cli
    push    dword 0ffffffffh            ; Dummy error code
    push    dword 0feba7632h            ; Vector Number
    jmp     dword int_handler_general

ALIGN   16
%endrep
;-------------------------------------------------------------------------------
;===============================================================================
