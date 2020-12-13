#! /bin/bash
## Change this to the root directory of the minus2C project ##
readonly PROJECT=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C
##                            ##                            ##
readonly TESTSFILE=$PROJECT/TESTING/TESTS.in
readonly MYCC=$PROJECT/mycc
readonly FILE=$PROJECT/TESTING/tac_solutions/tmp_tac
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