	.data
a: .word 2

	.text
	.globl main
third:
	# Push Caller Activation Record to stack
	# $s0 temporary storage for $ra
	sub $sp, $sp, 40
	sw $s0, 0($sp)
	sw d, 4($sp)
	sw $t0, 8($sp)
	sw $t1, 12($sp)
	sw $t2, 16($sp)
	sw $t3, 20($sp)
	sw $t4, 24($sp)
	sw $t5, 28($sp)
	sw $t6, 32($sp)
	sw $t7, 36($sp)

	# Function Body starts here
	li $t0,3
	sw $t0,d
	lw $t1,a
	lw $t2,b
	add $t3,$t1,$t2
	lw $t4,c
	add $t5,$t3,$t4
	lw $t6,d
	add $t7,$t5,$t6
	move $a0,$t7
	li $v0,17
	syscall
	# Function Body ends here

	# Restore Caller Activation Record from the stack
	lw $s0, 0($sp)
	lw d, 4($sp)
	lw $t0, 8($sp)
	lw $t1, 12($sp)
	lw $t2, 16($sp)
	lw $t3, 20($sp)
	lw $t4, 24($sp)
	lw $t5, 28($sp)
	lw $t6, 32($sp)
	lw $t7, 36($sp)
	add $sp, $sp, 40

main:
	# Push Caller Activation Record to stack
	# $s0 temporary storage for $ra
	sub $sp, $sp, 12
	sw $s0, 0($sp)
	sw first, 4($sp)
	sw $t0, 8($sp)

	# Function Body starts here

	move $s0, $ra
	jal first
	move $ra, $s0

	move $a0,$t0
	li $v0,17
	syscall
	# Function Body ends here

	# Restore Caller Activation Record from the stack
	lw $s0, 0($sp)
	lw first, 4($sp)
	lw $t0, 8($sp)
	add $sp, $sp, 12

first:
	# Push Caller Activation Record to stack
	# $s0 temporary storage for $ra
	sub $sp, $sp, 20
	sw $s0, 0($sp)
	sw b, 4($sp)
	sw second, 8($sp)
	sw $t0, 12($sp)
	sw $t1, 16($sp)

	# Function Body starts here
	li $t0,1
	sw $t0,b

	move $s0, $ra
	jal second
	move $ra, $s0

	move $a0,$t1
	li $v0,17
	syscall
	# Function Body ends here

	# Restore Caller Activation Record from the stack
	lw $s0, 0($sp)
	lw b, 4($sp)
	lw second, 8($sp)
	lw $t0, 12($sp)
	lw $t1, 16($sp)
	add $sp, $sp, 20

second:
	# Push Caller Activation Record to stack
	# $s0 temporary storage for $ra
	sub $sp, $sp, 16
	sw $s0, 0($sp)
	sw c, 4($sp)
	sw $t0, 8($sp)
	sw $t1, 12($sp)

	# Function Body starts here
	li $t0,2
	sw $t0,c

	move $s0, $ra
	jal third
	move $ra, $s0

