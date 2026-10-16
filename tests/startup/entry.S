.section .text.test_entry,"ax"
.global _test_entry
.align 2
_test_entry:
    mov.l .stack, r15
    mov.l .test, r0
    jsr @r0
    nop
    mov r0, r5
    mov #1, r4
    trapa #34
1:  bra 1b
    nop
.align 2
.stack: .long 0x00f00000
.test: .long _test_main
