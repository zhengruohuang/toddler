;===============================================================================
; Protected Mode Setup
;===============================================================================
;-------------------------------------------------------------------------------
; Constants for Protected Mode Setup
;-------------------------------------------------------------------------------
;       DA   : Descriptor Attribute
;       D    : Data segment
;       C    : Code segment
;       S    : System segment
;       R    : Read only
;       RW   : Read/Write
;       A    : Accessed
;-------------------------------------------------------------------------------
DA_32           equ     4000h   ;
DA_LIMIT_4K     equ     8000h   ;

DA_DPL0         equ     00h     ; DPL = 0
DA_DPL1         equ     20h     ; DPL = 1
DA_DPL2         equ     40h     ; DPL = 2
DA_DPL3         equ     60h     ; DPL = 3

SA_RPL0         equ     0       ;
SA_RPL1         equ     1       ;
SA_RPL2         equ     2       ;
SA_RPL3         equ     3       ;

SA_TIG          equ     0       ;
SA_TIL          equ     4       ;
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Segment
;-------------------------------------------------------------------------------
DA_DR           equ     90h     ;
DA_DRW          equ     92h     ;
DA_DRWA         equ     93h     ;
DA_C            equ     98h     ;
DA_CR           equ     9ah     ;
DA_CCO          equ     9ch     ;
DA_CCOR         equ     9eh     ;
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Flags in PDE & PTE
;-------------------------------------------------------------------------------
PG_P            equ     1       ; Whether the page exists
PG_RWR          equ     0       ; R/W Read&Execute
PG_RWW          equ     2       ; R/W Read&WRite&Execute
PG_USS          equ     0       ; U/S Supervisor
PG_USU          equ     4       ; U/S User
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; Macro: GdtDescriptor
;-------------------------------------------------------------------------------
; Usage: Descriptor Base, Limit, Attributes
;        Base:  dd
;        Limit: dd (low 20 bits available)
;        Attr:  dw (lower 4 bits of higher byte are always 0)
%macro GdtDescriptor 3
    dw      %2 & 0ffffh                         ; Limit1                 (2 Bytes)
    dw      %1 & 0ffffh                         ; Base1                  (2 Bytes)
    db      (%1 >> 16) & 0ffh                   ; Base2                  (1 Byte)
    dw      ((%2 >> 8) & 0f00h) | (%3 & 0f0ffh) ; Attr1 + Limit2 + Attr2 (2 Bytes)
    db      (%1 >> 24) & 0ffh                   ; Base3                  (1 Byte)
%endmacro                                       ; 8Bytes
;-------------------------------------------------------------------------------
;===============================================================================
