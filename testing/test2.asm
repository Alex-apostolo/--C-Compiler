    .globl main
    .text
main:
    li $t0,2
    li $t1,3
    add $t2, $t0, $t1
    move $a0,$t2
    li $v0,17
    syscall
