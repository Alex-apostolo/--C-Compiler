global a 2

proc main 0
load t0 125
store t0 x
load t1 35
store t1 y
closure func
t2 = call func
load t3 x
load t4 y
add t3 t4 t5
return t5

proc func 0
load t0 5
load t1 5
add t0 t1 t2
return t2
