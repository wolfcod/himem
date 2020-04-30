.MODEL TINY

.CODE

EXTRN   C Strategy
EXTRN   C XMSInterrupt

HEADER:
        dd      -1           ; Link to next driver, -1 end of list
        dw      0a000h       ; Device Attributes
        dw      Strategy     ; Strategy entry point
        dw      XMSInterrupt ; Interrupt Entry point
        db      'XMSXXX0'    ; DeviceName

        ret
END HEADER

