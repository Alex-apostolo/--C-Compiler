#! /bin/bash
readonly TESTSFILE=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/TESTING/TESTS.in
readonly MYCC=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/mycc
readonly FILE=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/TESTING/tac_solutions/tmp_tac
readonly OUTPUT=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/TESTING/tac_solutions/solutions.out
readonly RESULT=/Users/alex/SoftwareDevelopment/C/Compilers/minus2C/RESULT.tac

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
$MYCC -t $FILE
printf "\n\$\$\$ TEST $a: \$\$\$\n" >> $OUTPUT
cat $RESULT >> $OUTPUT
printf "\n"
a=$((a+1))
fi
done < $TESTSFILE
rm $FILE