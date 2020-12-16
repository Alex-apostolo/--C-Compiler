	.data

	.text
	.globl main

main:

	move $s0, $ra
	jal func
	move $ra, $s0

	move $a0,$t0
	li $v0,17
	syscall

sum:
	# Push Caller Activation Record to stack
	# $s0 temporary storage for $ra
	sub $sp, $sp, 16
	sw $s0, 0($sp)
	sw $t0, 4($sp)
	sw $t1, 8($sp)
	sw $t2, 12($sp)

	# Function Body starts here
	lw $t0,a
	lw $t1,b
	add $t2,$t0,$t1
	move $a0,$t2
	li $v0,17
	syscall
	# Function Body ends here

	# Restore Caller Activation Record from the stack
	lw $s0, 0($sp)
	lw $t0, 4($sp)
	lw $t1, 8($sp)
	lw $t2, 12($sp)
	add $sp, $sp, 16

func:
	# Push Caller Activation Record to stack
	# $s0 temporary storage for $ra
	sub $sp, $sp, 16
	sw $s0, 0($sp)
	sw $t0, 4($sp)
	sw $t1, 8($sp)
	sw $t2, 12($sp)

	# Function Body starts here
	lw $t0,c
	lw $t1,d
	add $t2,$t0,$t1
	move $a0,$t2
	li $v0,17
	syscall
	# Function Body ends here

	# Restore Caller Activation Record from the stack
	lw $s0, 0($sp)
	lw $t0, 4($sp)
	lw $t1, 8($sp)
	lw $t2, 12($sp)
	add $sp, $sp, 16
