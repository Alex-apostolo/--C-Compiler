    .globl main
    .text
main:
    jal function
    li $v0,10
    syscall
    
function:
    li $v0, 4
    la $a0, x 
    syscall 
    jr $ra

    .data
x: .asciiz "yo bro"
y: .asciiz "hello"