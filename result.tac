
proc main 0
closure func
t0 = call func
store t0 y
return y

proc func 0
closure func2
t0 = call func2
return t0

proc func2 0
load t0 5
store t0 x
return x
