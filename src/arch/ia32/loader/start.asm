;==============================================================================;
;                                                                              ;
; OS Loader for Toddler on IA32                                                ;
;                                                                              ;
;==============================================================================;


%include "./common/include/memlayout.inc"
%include "./loader/pm_setup.inc"


;===============================================================================
; Loader Entry
;===============================================================================
;We are now in real mode
; Load position
    org     LoaderOffsetSeg

; Jump to pre initialization and skip the following stuff
    jmp     PRE_INITIALIZATION
;===============================================================================


;===============================================================================
; For Protected Mode
;===============================================================================
;-------------------------------------------------------------------------------
; GDT Descriptors for Protected Mode
;-------------------------------------------------------------------------------
;           Base,           Limit,      Attributes  Description
GDT_START:  GdtDescriptor   0,          0,          0                               ; Dummy
GDT_RUN:    GdtDescriptor   0,          0fffffh,    DA_CR  | DA_32 | DA_LIMIT_4K    ; 0 - 4G
GDT_RW:     GdtDescriptor   0,          0fffffh,    DA_DRW | DA_32 | DA_LIMIT_4K    ; 0 - 4G
GDT_VIDEO:  GdtDescriptor   0b8000h,    0ffffh,     DA_DRW | DA_DPL3                ; Video
GDT_REAL16: GdtDescriptor   0,          0ffffh,     DA_C
GDT_REAL:   GdtDescriptor   0,          0ffffh,     DA_DRW
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Descriptor Info for Protected Mode
;-------------------------------------------------------------------------------
GdtLength       equ     $ - GDT_START           ; Length of GDT
GdtAddress      dw      GdtLength - 1           ; Limit
                dd      LoaderBase + GDT_START  ; Base
GdtAddress32    equ     LoaderBase + GdtAddress
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; GDT Selectors for Protected Mode
;-------------------------------------------------------------------------------
SelectorRun     equ     GDT_RUN    - GDT_START
SelectorRW      equ     GDT_RW     - GDT_START
SelectorVideo   equ     GDT_VIDEO  - GDT_START + SA_RPL3
SelectorReal16  equ     GDT_REAL16 - GDT_START
SelectorReal    equ     GDT_REAL   - GDT_START
;-------------------------------------------------------------------------------
;===============================================================================


;===============================================================================
; Variables
;===============================================================================
;-------------------------------------------------------------------------------
; What to Load
;-------------------------------------------------------------------------------
;   0 = Unknown
;   1 = Load HAL (Default)
;   2 = Start Application Processor
;   3 = Run BIOS Invoker
;-------------------------------------------------------------------------------
LoaderFuncType      dd      1
LoaderFuncType32    equ     LoaderBase + LoaderFuncType
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Application Processors
;-------------------------------------------------------------------------------
ApPageDirectoryPfn      dd      0
ApStartupEntry32        equ     LoaderBase + LoaderOffsetSeg
ApPageDirectoryPfn32    equ     LoaderBase + ApPageDirectoryPfn
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; HAL Entry
;-------------------------------------------------------------------------------
HalEntry        dd      0
HalEntry32      equ     LoaderBase + HalEntry
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Returnning to 16bit Real Mode
;-------------------------------------------------------------------------------
RealIdtr        dd      0
                dd      0
HalIdtr         dd      0
                dd      0
RealIdtr32      equ     LoaderBase + RealIdtr
HalIdtr32       equ     LoaderBase + HalIdtr
;-------------------------------------------------------------------------------
;===============================================================================


;ALIGN   4096
;===============================================================================
; Pre-Initialization
;===============================================================================
PRE_INITIALIZATION:
; Initialize segment registers and stack top
    mov     ax, LoaderBaseSeg
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     sp, LoaderStackTop

; Figure out what to load
    mov     eax, [LoaderFuncType]
    mov     dword [LoaderFuncType], 0

    ; Load HAL?
    cmp     eax, 1
    jz      START

    ; Run AP Initialization?
    cmp     eax, 2
    jz      AP_STARTUP_ENTRY
    
    ; BIOS Invoker?
    cmp     eax, 3
    jz      STOP

    ; Unknown or other?
    jmp     STOP
;===============================================================================


;===============================================================================
; Entry point of loading HAL
;===============================================================================
START:
.JumpToRealModeC:
; Call functions in C
    mov     edi, LoaderVariableStartOffset
    mov     dword [edi], .RealModeReturn

    mov     ebx, LoaderRealSetupEntry
    jmp     ebx
        
.RealModeReturn:
; Move to protected mode
    xor     eax, eax
    xor     edx, edx

    ; Save Real Mode IDTR
    sidt    [RealIdtr]

    ; Load GDT
    lgdt    [GdtAddress]

    ; Disable interrupt
    cli

    ; Prepare to move to protected mode
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax

    ; Jump to 32-bit program
    jmp     dword SelectorRun : (LoaderBase + START_32)

    ; 16-bit code will stop here, and the program will continue at START_32
    jmp     STOP
    
    ; Should never reach here
    jmp     $
;===============================================================================


;===============================================================================
; Stop the OS loader
;===============================================================================
STOP:
    cli
    hlt
    jmp     STOP
;===============================================================================


;===============================================================================
; Application processor 16-bit entry point
;===============================================================================
AP_STARTUP_ENTRY:
; 16-bit code: move to protected mode
    cli

    lgdt    [GdtAddress]

    ; Prepare to move to the Protected Mode
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax

    ; Jump to 32-bit program
    jmp     dword SelectorRun : (LoaderBase + AP_STARTUP_ENTRY_32)
;===============================================================================


;===============================================================================
; BIOS invoker 16-bit: make a BIOS call, then go back to 32-bit protected mode
;===============================================================================
ALIGN   32

;-------------------------------------------------------------------------------
; This is in 16-bit protected mode, we will move to 16-bit real mode
;-------------------------------------------------------------------------------
BIOS_INVOKER_ENTRY_16:
    ; Move back to real mode
    mov     ax, SelectorReal
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    ; Disable protected mode
    mov     eax, cr0
    and     al, 11111110b
    mov     cr0, eax

    ; Jump to 16-bit real mode
    jmp     LoaderBaseSeg : BIOS_INVOKER_ENTRY
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Make a BIOS Call
;-------------------------------------------------------------------------------
BIOS_INVOKER_ENTRY:
    ; Disable interrupts
    cli

    xor     eax, eax
    mov     ax, cs
    mov     ds, ax
    mov     ss, ax

    mov     esp, 06000h

    ; Get Interrupt Number
    pop     eax
    mov     byte [IntVec], al

    ; Pop All Registers
    popad

; This is an int instruction
.BiosInvokerIntInstruction:
    ; Opcode
    Opcode  db  0xcd
    ; Interrupt Number
    IntVec  db  0

    ; Save the result of interrupt
    pushad

    ; Save Interrupt Number
    xor     eax, eax
    mov     byte al, [IntVec]
    push    eax

;We have successfully made a BIOS call, now we are going back to protected mode
.BiosInvokerIntInstructionToProtectedMode:
    ; Load GDT
    lgdt    [GdtAddress]

    ; Prepare to move to Protected Mode
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax

    ; Jump to 32-bit program
    jmp     dword SelectorRun : (LoaderBase + BIOS_INVOKER_RETURN_32)
;-------------------------------------------------------------------------------
;===============================================================================



[SECTION .s32]

ALIGN      32

[BITS      32]
;===============================================================================
; 32-bit Program
;===============================================================================
; 32-bit protected code starts here
START_32:
; Initialize segment registers
    mov     ax, SelectorRW
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    mov     esp, LoaderStackTop32

; Call functions in C
.JumpToProtectedModeC:
    ; Set up return address
    mov     edi, LoaderBase + LoaderVariableStartOffset
    mov     dword [edi], .ProtectedModeReturn + LoaderBase

    ; Set up the address for the variables that holds the address of HAL entry
    mov     edi, LoaderBase + LoaderVariableStartOffset + 4
    mov     dword [edi], HalEntry32

    ; Set up the address of LoaderFuncType32
    mov     edi, LoaderBase + LoaderVariableStartOffset + 8
    mov     dword [edi], LoaderFuncType32

    ; ApStartupEntry32
    mov     edi, LoaderBase + LoaderVariableStartOffset + 12
    mov     dword [edi], ApStartupEntry32

    ; BIOS_INVOKER_ENTRY_32
    mov     edi, LoaderBase + LoaderVariableStartOffset + 16
    mov     dword [edi], LoaderBase + BIOS_INVOKER_ENTRY_32

    ; Jump to C entry
    mov     ebx, LoaderProtectedSetupEntry
    jmp     ebx

.ProtectedModeReturn:
; Jump to the entry of HAL
; EDI is filled with the entry addr of HAL by the C code
    mov     ebx, edi
    call    ebx     ; Jump to HAL, and the Loader should finish its task now
    
; Stop
    jmp     STOP_32
    
; Should never reach here
    jmp     $
;===============================================================================


;===============================================================================
; Stop the OS Loader 32-bit
;===============================================================================
STOP_32:
    cli
    hlt
    jmp     short STOP_32
;===============================================================================


;===============================================================================
; Application processor entry 32-bit: jump to HAL with special parameters
;===============================================================================
AP_STARTUP_ENTRY_32:
; Setup segment registers
    xor     eax, eax

    mov     ax, SelectorRW
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    mov     esp, LoaderStackTop32

; Setup paging
    mov     eax, [ApPageDirectoryPfn32]       ;  256

    cmp     eax, 0
    jz      STOP_32

    shl     eax, 12
    mov     cr3, eax
    mov     eax, cr0
    or      eax, 80000000h
    mov     cr0, eax
    jmp     short .EnablePagedAddressApStartup               ; Force to use the new address space

.EnablePagedAddressApStartup:
    nop

; Jump to MP initialization Entry in HAL
    mov     ebx, [HalEntry32]
    jmp     ebx

; Failed
    jmp     STOP_32
    
; Should never reach here
    jmp     $
;===============================================================================


;===============================================================================
; BIOS Invoker 32-Bit
;===============================================================================
;-------------------------------------------------------------------------------
; Save Execution Environment and Move to 16-bit Protected Mode
;-------------------------------------------------------------------------------
BIOS_INVOKER_ENTRY_32:
; Disable interupts
    cli

; Save HAL's IDTR
    sidt    [HalIdtr32]

; Move to 16-bit protected mode
    lgdt    [GdtAddress32]

    ; Set segment resiters
    jmp     dword SelectorRun : (LoaderBase + .SetSegmentRegisters)
.SetSegmentRegisters:
    mov     ax, SelectorRW
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    ; Disable Paging
    mov     eax, cr0
    and     eax, 7FFFFFFFh
    mov     cr0, eax
    jmp     short .DisablePagedAddressBiosInvoker

.DisablePagedAddressBiosInvoker:
    nop

    ; Jump to 16-bit protected mode
    jmp     word SelectorReal16 : (LoaderBase + BIOS_INVOKER_ENTRY_16)

;Shoudl Never Arrive Here
    jmp     STOP_32

    hlt
    jmp     $
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Restore Execution Environment and Return to HAL
;-------------------------------------------------------------------------------
BIOS_INVOKER_RETURN_32:
; Setup segment registers
    xor     eax, eax

    mov     ax, SelectorRW
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    mov     esp, LoaderStackTop32

; Setup paging
    mov     eax, [ApPageDirectoryPfn32]

    cmp     eax, 0
    jz      STOP_32

    shl     eax, 12
    mov     cr3, eax
    mov     eax, cr0
    or      eax, 80000000h
    mov     cr0, eax
    jmp     short .EnablePagedAddressBiosInvoker

.EnablePagedAddressBiosInvoker:
    nop

; Restore IDTR
    lidt    [HalIdtr32]

; Jump to HAL
    mov     ebx, [HalEntry32]
    jmp     ebx

; Failed
    jmp     STOP_32

; Should never reach here
    jmp     $
;-------------------------------------------------------------------------------
;===============================================================================


ALIGN   1024
