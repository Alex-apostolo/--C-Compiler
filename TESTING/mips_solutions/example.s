    .globl main
    .text
main:
    sub $sp, $sp, 8
    sw $s0, 0($sp)
    sw $t0, 4($sp)
    jal func
    lw $s0, 0($sp)
    add $sp, $sp, 4
func: