
proc main 0
block
load t6 4
store t6 a
t7 = call factorial
return t7
endblock 1


proc factorial 1
block
load t0 a
load t1 a
load t2 1
sub t1 t2 t3
store t3 a
t4 = call factorial
mul t0 t4 t5
store t5 a
return a
endblock 1

