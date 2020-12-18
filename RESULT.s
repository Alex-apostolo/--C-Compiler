	.data
a: .word 2
x:
y:

	.text
	.globl main

main:
	li $t0,125
	sw $t0,x
	li $t1,35
	sw $t1,y

	# Push t registers
	add $sp, $sp, -24
	sw $t0, 0($sp)
	sw $t1, 4($sp)
	sw $t2, 8($sp)
	sw $t3, 12($sp)
	sw $t4, 16($sp)
	sw $t5, 20($sp)
	# Push locals
	add $sp, $sp, -8
	lw $t0, x
	sw $t0, 0($sp)
	lw $t1, y
	sw $t1, 4($sp)
	 # Jump instruction
	jal func

	lw $t3,x
	lw $t4,y
	add $t5,$t3,$t4
	move $a0,$t5
	li $v0,17
	syscall

func:
	# Push $ra and old $fp
	add $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	# New frame pointer
	add $fp, $sp, 8

	li $t0,5
	li $t1,5
	add $t2,$t0,$t1

	# Restore locals
	lw $t0, 0($fp)
	sw $t0, x
	lw $t1, 4($fp)
	sw $t1, y
	# Restore t registers
	lw $t0, 8($fp)
	lw $t1, 12($fp)
	lw $t2, 16($fp)
	lw $t3, 20($fp)
	lw $t4, 24($fp)
	lw $t5, 28($fp)
	# Restore $ra
	lw $ra, -8($fp)
	# Restore $fp
	lw $fp, -4($fp)
	# Restore $sp
	add $sp, $sp, 40
	 # Jump instruction
	move $v0,$t2
	jr $ra
