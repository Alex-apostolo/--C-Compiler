    .data
x: .word 15
y: .ascii "hello"

    .globl main
main:
    jal func

func:
    li $a0, 69
    li $v0, 1
    syscall
    jr $ra