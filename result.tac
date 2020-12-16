
proc main 0
block 3
closure sum
closure func
t0 = call func
return t0

proc sum 2
block 3
load t1 a
load t2 b
add t1 t2 t3
return t3

proc func 2
block 3
load t4 c
load t5 d
add t4 t5 t6
return t6
