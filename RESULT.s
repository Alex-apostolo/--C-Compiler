	.data
a: .word 0
e: .word 0
c: .word 0
d: .word 0

	.text
	.globl main

main:
	li $t0,5
	sw $t0,a
	li $t1,90
	sw $t1,e

	# Push t registers
	add $sp, $sp, -12
	sw $t0, 0($sp)
	sw $t1, 4($sp)
	sw $t2, 8($sp)
	 # Jump instruction
	jal sum

	sw $v0,c
	li $t2,35
	sw $t2,d

	# Push t registers
	add $sp, $sp, -12
	sw $t0, 0($sp)
	sw $t1, 4($sp)
	sw $t2, 8($sp)
	 # Jump instruction
	jal func

	move $a0, $v0
	li $v0,17
	syscall

sum:
	# Push $ra and old $fp
	add $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	# New frame pointer
	add $fp, $sp, 8

	lw $t0,a
	lw $t1,e
	add $t2,$t0,$t1
	move $v0, $t2

	# Restore t registers
	lw $t0, 0($fp)
	lw $t1, 4($fp)
	lw $t2, 8($fp)
	# Restore $ra
	lw $ra, -8($fp)
	# Restore $fp
	lw $fp, -4($fp)
	# Restore $sp
	add $sp, $sp, 20
	 # Jump instruction
	jr $ra

func:
	# Push $ra and old $fp
	add $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	# New frame pointer
	add $fp, $sp, 8

	lw $t0,c
	lw $t1,d
	add $t2,$t0,$t1
	move $v0, $t2

	# Restore t registers
	lw $t0, 0($fp)
	lw $t1, 4($fp)
	lw $t2, 8($fp)
	# Restore $ra
	lw $ra, -8($fp)
	# Restore $fp
	lw $fp, -4($fp)
	# Restore $sp
	add $sp, $sp, 20
	 # Jump instruction
	jr $ra
