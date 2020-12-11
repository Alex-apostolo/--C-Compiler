
proc main 0
block
load t6 5
store t6 a
load t7 90
store t7 b
t8 = call sum
store t8 c
load t9 35
store t9 d
t10 = call func
return t10
endblock 2


proc sum 2
block
load t0 a
load t1 b
add t0 t1 t2
return t2
endblock 2


proc func 2
block
load t3 c
load t4 d
add t3 t4 t5
return t5
endblock 2

