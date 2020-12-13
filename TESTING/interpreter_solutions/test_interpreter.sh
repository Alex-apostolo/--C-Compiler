#! /bin/bash
## Change this to the root directory of the minus2C project ##
readonly PROJECT=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C
##                            ##                            ##
readonly RESULT=$PROJECT/TESTING/interpreter_solutions/solutions.out
readonly SOLPATH=$PROJECT/TESTING/interpreter_solutions/solutions.in
readonly TESTSFILE=$PROJECT/TESTING/TESTS.in
readonly MYCC=$PROJECT/mycc

rm $RESULT
a=1
b=1
printf "Start Testing at: "
date
printf "\n### START OF OUTPUT ###\n\n"
while read line
do 
if [ "$line" != "%%" ] 
then
$MYCC -p <<< $line > /dev/null
str="TEST $a, LINE $b: Terminates with -> $?"
echo $str
echo $str >> $RESULT

a=$((a+1))
b=$((b+1))
fi
done < $TESTSFILE

printf "\n### END OF OUTPUT ###\n\n*** DIFFERENCES ***\n\n"

diff $RESULT $SOLPATH

if [ $? = 0 ]
    then 
    printf "All tests passed successfully!!!\n\n"
    else
    printf "\nSome or all tests Failed ;(\n\n"
fi