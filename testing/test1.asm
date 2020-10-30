    .globl main
    .text
main:
    li $t0,6
    sw $t0,x
    lw $t1,x
    move $a0, $t1
    li $v0,1
    syscall
    li $v0,17
    syscall

    .data
x: .word 69
