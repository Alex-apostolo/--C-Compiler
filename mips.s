	.globl main
	.text
main:
	li $t0,1
	sw $t0,x
	lw $t1,x
	li $t2,32
	add $t3,$t1,$t2
	sw $t3,x
	lw $t4,x
	move $a0,$t4
	li $v0,17
	syscall
	.data
x: 
