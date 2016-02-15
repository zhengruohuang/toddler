;==============================================================================;
;                                                                              ;
; Floppy Boot Sector for Toddler on IA32                                       ;
;                                                                              ;
;==============================================================================;


;===============================================================================
; Include
;===============================================================================
%include "./common/include/memlayout.inc"
;===============================================================================


;===============================================================================
; Constants
;===============================================================================
CEntry      equ     BootEntry + 512
;===============================================================================


;===============================================================================
; Entry point
;===============================================================================
; Load position
    org     BootEntry
Entry:
    jmp     0 : Initialize
;===============================================================================


;===============================================================================
; Initialization
;===============================================================================
Initialize:
; Registers for segments
    mov     ax, cs
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     sp, BootStackTop

; Start
    jmp     Start
;===============================================================================


;===============================================================================
; Strings
;===============================================================================
WelcomeMessage      db      "Welcome to Toddler!", 0
LoadMessage         db      "Loading boot program ...", 0
ErrorMessage        db      "Unable to boot Toddler!", 0
DoneMessage         db      " Done!", 0
;===============================================================================


;===============================================================================
; Start Booting
;===============================================================================
Start:
; Clear the screen
    mov     ax, 0600h       ; ah = 6,  al = 0h
    mov     bx, 0700h       ; Backcolor: Black, Forecolor: White (BL = 07h)
    mov     cx, 0           ; (Left, Top): (0, 0)
    mov     dx, 184fh       ; (Right, Bottom): (25, 80)
    int     10h             ; Call int 10h to clear the screen

; Set the position of the cursor
    mov     ah, 02h
    mov     bh, 00h
    mov     dx, 0000h
    int     10h

; Show welcome message
    mov     dx, WelcomeMessage
    call    ShowText
    call    NewLine
    call    NewLine

; Load boot program: floppy sector 1 to 15
    mov     dx, LoadMessage
    call    ShowText

    mov     ax, 1
    mov     cx, 15
    xor     ebx, ebx
    mov     es, bx
    mov     bx, CEntry
    call    ReadFile

; Call the start function in C
.CallFunctionInC:
    mov     dx, DoneMessage
    call    ShowText
    call    NewLine

    mov     ebx, CEntry
    jmp     ebx

; Should never arrive here
Error:
    mov     dx, ErrorMessage
    call    ShowText
    call    NewLine

; Stop
Stop:
    jmp     $
;===============================================================================


;===============================================================================
;           Functions
;-------------------------------------------------------------------------------
;   void print_char(char c in al)
;-------------------------------------------------------------------------------
;   Draw a charactor on the screen.
;   c: The charactor to draw.
;      This param is passed by
;       the register al.
;-------------------------------------------------------------------------------
PrintChar:
; Save register values
    push    ax
    push    bx
    push    cx
    push    dx

; Draw the char on screen
    mov     ah, 0eh
    mov     bh, 0
    int     10h

; Restore register values
    pop     dx
    pop     cx
    pop     bx
    pop     ax

; Return
    ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;   void print_string(char *s in dx)
;-------------------------------------------------------------------------------
;   Draw some text on the screen.
;   s: The text to draw.
;      This param is passed by
;       the register DX.
;-------------------------------------------------------------------------------
ShowText:
; Save register values
    push    ax          ; Save the value of the registers
    push    bp

; Copy
    mov     bp, dx      ; Copy the pointer of the string from dx to bp

; Draw a char
.Draw:
    cmp     byte [bp], 0h   ; Compare whether we reaches the end of the string
    jz      .DrawFinished   ; If the drawing is finished, jump to .DrawFinished
    mov     al, [bp]    ; Else, set the char to paint
    call    PrintChar   ; Call print_char

; Move to the next char
    inc     bp          ; Make the pointer point to the next char
    jmp     .Draw       ; Loop

; Finish the drawing (restore register values)
.DrawFinished:
    pop     bp          ; Restore the value of registers
    pop     ax

; Return
    ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;   void new_line()
;-------------------------------------------------------------------------------
;   Move the cursor to the next line.
;-------------------------------------------------------------------------------
NewLine:
    push    ax          ; Save the value of the registers
    push    bx
    push    cx
    push    dx

    mov     ax, 0300h   ; Get the current cursor position
    mov     bx, 0000h   ; Set the page number to 0
    int     10h         ; Call the BIOS to get the position of the cursor

    mov     ax, 0200h   ; Set the position of the cursor
    add     dx, 100h    ; Increase the value of dh, which means the line number
    mov     dl, 0h      ; Set the cursor to the first column
    int     10h         ; Call the BIOS to set the position of the cursor

    pop     dx          ; Restore the value of the registers
    pop     cx
    pop     bx
    pop     ax

    ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;   void read_file(int16 sec_num in ax, int16 sec_count in cx,
;                  void *buffer in es:bx)
;-------------------------------------------------------------------------------
;   sec_num: Starting sector,
;   sec_count: Sector count
;   buffer: Where to load (es:bx = seg : offset)
;-------------------------------------------------------------------------------
ReadFile:
    push    eax
    push    ebx
    push    ecx

.ReadAndPrintDot:
    call    ReadSector

    inc     ax
    add     ebx, 512

.PrintDot:
    push    ax
    mov     al, '.'
    call    PrintChar
    pop     ax

    loop    .ReadAndPrintDot

    pop     ecx
    pop     ebx
    pop     eax

    ret
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
;   void read_sector(int16 sec_num in ax, int16 load_to in es:bx)
;-------------------------------------------------------------------------------
;   Read a sector to a specific position.
;   sec_num: The number of the sector that you want to read.
;        This param is passed through register AX.
;   load_to: The address that the content of the sector will be stored.
;        This parameter is passed through register ES:BX.
;-------------------------------------------------------------------------------
ReadSector:
    push    ax
    push    cx
    push    dx

; Calculate cylinder, sector, and head by LBA
    push    bx          ; Save bx
    mov     bl, 18      ; bl is divisor
    div     bl          ; y is in al, z is in ah
    inc     ah          ; z ++
    mov     cl, ah      ; cl <- Starting Sector Number
    mov     dh, al      ; dh <- y
    shr     al, 1       ; y >> 1 (which means y/BPB_NumHeads, BPB_NumHeads=2)
    mov     ch, al      ; ch <- Cylinder Number
    and     dh, 1       ; dh & 1 = Header Number
    pop     bx          ; Restore bx

.KeepRead:
    mov     dl, 0       ; The device number that you wish to read (0 means A:\ in DOS)
    mov     ah, 2       ; Set the function number
    mov     al, 1       ; Load only onew sector
    int     13h         ; Call the BIOS to read a sector
    jc      .KeepRead   ; If there are errors, CF will be set to 1,
                        ; we will continue reading until it errors are corrected

    pop     dx
    pop     cx
    pop     ax

    ret
;-------------------------------------------------------------------------------
;===============================================================================


;===============================================================================
; End of Boot Sector: Magic Number
;===============================================================================
    times   510 - ($ - $$)  db 0    ; Fill the rest sapce, in order to make the file size 512B
    dw      0xaa55                  ; Magic number
;===============================================================================
