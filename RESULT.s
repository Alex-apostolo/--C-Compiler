	.data

	.text
	.globl main

main:

	 # Jump instruction
	jal func

	move $a0, $v0
	li $v0,17
	syscall

func:
	# Push $ra and old $fp
	add $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	# New frame pointer
	add $fp, $sp, 8

	lw $t0,a
	lw $t1,b
	add $t2,$t0,$t1

	# Restore $ra
	lw $ra, -8($fp)
	# Restore $fp
	lw $fp, -4($fp)
	# Restore $sp
	add $sp, $sp, 8
	 # Jump instruction
	move $v0,$t2
	jr $ra
