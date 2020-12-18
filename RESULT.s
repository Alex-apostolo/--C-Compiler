	.data
y:
x:

	.text
	.globl main

main:

	# Push t registers
	add $sp, $sp, -4
	sw $t0, 0($sp)
	 # Jump instruction
	jal func

	sw $t0,y
	lw $t0, y
	move $a0, $t0
	li $v0, 17
	syscall

func:
	# Push $ra and old $fp
	add $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	# New frame pointer
	add $fp, $sp, 8


	# Push t registers
	add $sp, $sp, -4
	sw $t0, 0($sp)
	 # Jump instruction
	jal func2


	# Restore t registers
	lw $t0, 0($fp)
	# Restore $ra
	lw $ra, -8($fp)
	# Restore $fp
	lw $fp, -4($fp)
	# Restore $sp
	add $sp, $sp, 12
	 # Jump instruction
	move $v0,$t0
	jr $ra

func2:
	# Push $ra and old $fp
	add $sp, $sp, -8
	sw $ra, 0($sp)
	sw $fp, 4($sp)
	# New frame pointer
	add $fp, $sp, 8

	li $t0,5
	sw $t0,x

	# Restore t registers
	lw $t0, 0($fp)
	# Restore $ra
	lw $ra, -8($fp)
	# Restore $fp
	lw $fp, -4($fp)
	# Restore $sp
	add $sp, $sp, 12
	 # Jump instruction
	lw $t0, x
	move $v0, $t0
	jr $ra
