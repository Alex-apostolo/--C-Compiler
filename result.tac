
proc main 0
closure sum
closure func
load t0 5
store t0 a
load t1 90
store t1 e
v0 = call sum
store v0 c
load t2 35
store t2 d
v0 = call func
return v0

proc sum 2
load t0 a
load t1 e
add t0 t1 t2
return t2

proc func 2
load t0 c
load t1 d
add t0 t1 t2
return t2
