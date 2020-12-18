global a 2

proc main 0
closure first
closure third
v0 = call first
return v0

proc first 0
load t0 1
store t0 e
closure second
v0 = call second
return v0

proc third 0
load t0 a
load t1 e
add t0 t1 t2
load t3 c
add t2 t3 t4
return t4

proc second 0
load t0 2
store t0 c
v0 = call third
return v0
