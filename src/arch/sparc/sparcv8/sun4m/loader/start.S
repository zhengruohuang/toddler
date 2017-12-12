.global start

.section BOOTSTRAP, "ax"

start:
    set 0x3f00, %sp
    
    call loader_entry
    nop
    
    b .
    nop
