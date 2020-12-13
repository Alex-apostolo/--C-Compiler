#! /bin/bash
## Change this to the root directory of the minus2C project ##
readonly PROJECT=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C
##                            ##                            ##
readonly TESTSFILE=$PROJECT/TESTING/TESTS.in
readonly MYCC=$PROJECT/mycc
readonly FILE=$PROJECT/TESTING/mips_solutions/tmp_tac
readonly OUTPUT=$PROJECT/TESTING/mips_solutions/solutions.out
readonly RESULT=$PROJECT/RESULT.s

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
$MYCC -s $FILE
printf "\n\$\$\$ TEST $a, LINE $b: \$\$\$\n\n" >> $OUTPUT
cat $RESULT >> $OUTPUT
printf "\n\n"
a=$((a+1))
b=$((b+2))
fi
done < $TESTSFILE
rm $FILE