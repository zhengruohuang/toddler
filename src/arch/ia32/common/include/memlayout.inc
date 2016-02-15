;-------------------------------------------------------------------------------
; Boot
;-------------------------------------------------------------------------------
BootBaseSeg                     equ     0
BootEntry                       equ     07c00h

BootStackTop                    equ     07c00h

BootSectorBufferOffset1         equ     05000h
BootSectorBufferOffset2         equ     06000h
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; Loader Position
;-------------------------------------------------------------------------------
; The position of Loader.bin should be loaded in base:offset format
LoaderBaseSeg                   equ     00000h
LoaderOffsetSeg                 equ     0b000h

; The position of Loader.bin should be loaded in linear format
LoaderBase                      equ     LoaderBaseSeg * 010h

LoaderStackTop                  equ     0b000h
LoaderStackTop32                equ     LoaderBase + LoaderStackTop

LoaderRealSetupEntry            equ     0b800h
LoaderProtectedSetupEntry       equ     LoaderBase + 0d000h

LoaderVariableStartOffset       equ     0f000h
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; Core Image Position
;-------------------------------------------------------------------------------
CoreLoadBase                    equ     01000h
CoreLoadOffset                  equ     00000h

CoreLoadAddress32               equ     CoreLoadBase * 010h
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; Used by Loader
;-------------------------------------------------------------------------------
; PageTableStartAddress32         equ     100000h         ; 1Mb
; PageTableStartPfn32             equ     256
; 
; HalPteLow4MPfn32                equ     257
; HalPteHalImgPfn32               equ     258
; 
; IsaZoneSizeTimes4Mb32           equ     3               ; 3 * 4MB = 12MB
;-------------------------------------------------------------------------------


;-------------------------------------------------------------------------------
; Used by HAL
;-------------------------------------------------------------------------------
; HalStackTop                     equ     0FFC02000h
; InteruptStackTop                equ     0FFC04000h
; FastSysCallStackTop             equ     0FFC06000h
;-------------------------------------------------------------------------------
