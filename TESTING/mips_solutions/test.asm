	.data
x:

	.text
	.globl main

main:
	li $t0,1
	sw $t0,x
	lw $t1,x
	li $t2,32
	add $t3,$t1,$t2
	sw $t3,x