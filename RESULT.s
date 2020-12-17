	.data

	.text
	.globl main

main:

	# Push t registers
	add $sp, $sp, -4
	sw $t0, 0($sp)
	 # Jump instruction
	jal func

	move $a0,$t0
	li $v0,17
	syscall

sum:
	# Push $ra and old $fp
	add $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	# New frame pointer
	add $fp, $sp, -8

	lw $t0,a
	lw $t1,b
	add $t2,$t0,$t1

	# Restore $ra
	lw $ra, -4($fp)
	# Restore t registers
	lw $t0, 4($fp)
	 # Jump instruction
	move $a0,$t2
	li $v0,17
	syscall

func:
	# Push $ra and old $fp
	add $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	# New frame pointer
	add $fp, $sp, -8

	lw $t0,c
	lw $t1,d
	add $t2,$t0,$t1

	# Restore $ra
	lw $ra, -4($fp)
	# Restore t registers
	lw $t0, 4($fp)
	 # Jump instruction
	move $a0,$t2
	li $v0,17
	syscall
