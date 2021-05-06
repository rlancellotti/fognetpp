#!/bin/bash
#INIFILE="RLomnetpp.ini"
#INIFILE="testServerTS.ini"
INIFILE="$1"
OUTFILE="Runfile"
SLEEPTIME=1
FAMILY="."

MAXRUN=`grep '^repeat.*=' $INIFILE | cut -d '=' -f2 | head -n1`
((MAXRUN=MAXRUN-1))



if [[ -n "$2" ]]
then
	FAMILY="$2"
fi

#CONFIGS="MGnDanailo-0 MGnDanailo-1 MGnDanailo-2"
CONFIGS=$(cat ${INIFILE} | grep '\[Config' | sed 's/[][]//g' | grep -v Base | grep -e ${FAMILY}| cut -d' ' -f2)
#CONFIGS=`grep '\[Config' $INIFILE | sed 's/\[//g' | sed 's/\]//g' | cut -d' ' -f2 | grep -v MGnBase | grep -v 'MGnDanailo$'`
#echo $CONFIGS
echo preparing `echo $CONFIGS | wc -w` configs with 0..$MAXRUN runs each
#exit
TMPFILE="/tmp/$$.tmp"
RUNS=`seq 0 $MAXRUN`
echo > $TMPFILE
rm -f $OUTFILE
RUNNAMES=""
for c in $CONFIGS
do
	for r in $RUNS
	do
		#FLAGFILE="c${c}_r${r}.flag"
		SCANAME="results/${c}-\#${r}.sca"
		RUNNAME=$SCANAME
		RUNNAMES="$RUNNAMES $RUNNAME"
		echo "${SCANAME}: ${INIFILE}" >> $TMPFILE
		echo -e "\trm -f ${SCANAME}" `echo $SCANAME| sed s/sca/vec/` `echo $SCANAME| sed s/sca/vci/` >> $TMPFILE
		echo -e "\t./run -u Cmdenv -c $c -r $r -f $INIFILE" --cmdenv-performance-display=false --cmdenv-status-frequency=60s -s >> $TMPFILE
		#echo -e "\ttouch ${FLAGFILE}" >> $TMPFILE
		echo -e "\tsleep ${SLEEPTIME}" >> $TMPFILE
		#echo -e "\tsync" >> $TMPFILE
	done
done
#echo ".PHONY: ${RUNNAMES}" >> $OUTFILE
#echo "" >> $OUTFILE
echo "all: ${RUNNAMES}" >> $OUTFILE
cat $TMPFILE >> $OUTFILE
rm -f $TMPFILE

echo "" >> $OUTFILE
echo "clean:" >> $OUTFILE
echo -e "\trm -f results/*.sca results/*.vec results/*.vci" >> $OUTFILE

