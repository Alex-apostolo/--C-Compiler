#! /bin/bash
## Change this to the root directory of the minus2C project ##
readonly PROJECT=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C
##                            ##                            ##
readonly TESTSFILE=$PROJECT/TESTING/TESTS.in
readonly MYCC=$PROJECT/mycc
readonly FILE=$PROJECT/TESTING/tac_solutions/tmp_tac
readonly SOLPATH=$PROJECT/TESTING/tac_solutions/solutions.in
readonly OUTPUT=$PROJECT/TESTING/tac_solutions/solutions.out
readonly RESULT=$PROJECT/RESULT.tac

rm $OUTPUT
a=1
b=1
printf "Start Testing at: "
date
printf "\n"
while read line
do 
if [ "$line" != "%%" ] 
then
echo -n $line > $FILE
$MYCC -t $FILE
printf "\n\$\$\$ TEST $a, LINE $b: \$\$\$\n" >> $OUTPUT
cat $RESULT >> $OUTPUT
printf "\n"
a=$((a+1))
b=$((b+2))
fi
done < $TESTSFILE
rm $FILE

printf "\n### END OF OUTPUT ###\n\n*** DIFFERENCES ***\n\n"

diff $OUTPUT $SOLPATH

if [ $? = 0 ]
    then 
    printf "All tests passed successfully!!!\n\n"
    else
    printf "\nSome or all tests Failed ;(\n\n"
fi