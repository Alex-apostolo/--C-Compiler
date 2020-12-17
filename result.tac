
proc main 0
closure sum
closure func
t0 = call func
return t0

proc sum 2
load t0 a
load t1 b
add t0 t1 t2
return t2

proc func 2
load t0 c
load t1 d
add t0 t1 t2
return t2
