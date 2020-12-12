#! /bin/bash
readonly TESTSFILE=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/TESTING/INTERPRETER_TESTS/TESTS.in
readonly MYCC=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/mycc
readonly FILE=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/TESTING/MIPS_TESTS/tmp_tac
readonly OUTPUT=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/TESTING/MIPS_TESTS/solutions.out
readonly RESULT=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/RESULT.s

rm $OUTPUT
a=1
printf "Start Testing at: "
date
printf "\n"
while read line
do 
if [ "$line" != "%%" ] 
then
echo -n $line > $FILE
$MYCC -s $FILE
printf "\n\$\$\$ TEST $a: \$\$\$\n\n" >> $OUTPUT
cat $RESULT >> $OUTPUT
printf "\n\n"
a=$((a+1))
fi
done < $TESTSFILE
rm $FILE