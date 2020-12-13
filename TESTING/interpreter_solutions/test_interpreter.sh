#! /bin/bash
readonly RESULT=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/TESTING/interpreter_solutions/solutions.out
readonly SOLPATH=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/TESTING/interpreter_solutions/solutions.in
readonly TESTSFILE=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/TESTING/TESTS.in
readonly MYCC=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/mycc

rm $RESULT
a=1
printf "Start Testing at: "
date
printf "\n### START OF OUTPUT ###\n\n"
while read line
do 
if [ "$line" != "%%" ] 
then
$MYCC -p <<< $line > /dev/null
str="$a: Terminates with -> $?"
echo $str
echo $str >> $RESULT

a=$((a+1))
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