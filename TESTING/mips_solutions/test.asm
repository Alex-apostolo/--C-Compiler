    .data

	.text
	.globl main

main:
	li $a0, 5
    li $a1, 32

    li $t0, 139
    li $t1, 69

    # Push arguments
    add $sp, $sp, -8
    sw $a0, 0($sp)
    sw $a1, 4($sp)
    # Push temporaries
    add $sp, $sp, -8
    sw $t0, 0($sp)
    sw $t1, 4($sp)
    # Function call
    jal func
	syscall

func:
    # Push Old Frame Pointer and Return address
    add $sp, $sp, -8
    sw $fp, 0($sp)
    sw $ra, 4($sp)
    # New Frame pointer
    add $fp, $sp, -8



    # Restore $ra
    lw $ra, 0($fp)
    # Restore temporaries
    lw $t0, 4($fp)
    lw $t1, 8($fp)
    # Restore arguments
    lw $a0, 12($fp)
    lw $a1, 16($fp)
    # Restore old frame pointer and return address
    lw $sp, $fp
    lw $fp, 4($fp)
    # Jump to caller
    jr $ra


