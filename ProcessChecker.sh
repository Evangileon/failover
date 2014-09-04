#!/bin/bash

#use 'echo $?' command after the program terminates to find the status


count=0
firstLine=0
initialCount=0

PROCLIST=(
   "/usr/sbin/asterisk"
   "/usr/sbin/safe_asterisk"
)

initialCount=${#PROCLIST[@]}

for proc in ${PROCLIST[*]}; do

   PCOUNT=$(ps aux | grep $proc | grep -v grep | wc -l)
      
   if [ $PCOUNT -ne 0 ]; then
      c=$(ps aux | grep $proc | grep -v grep | awk ' { print $2 } ')
      echo $PCOUNT process found with the name $proc with PID $c
      count=$(($count + $PCOUNT))
   else
      echo no process found with the name $proc
   fi
done

echo "last count $count"
count=$(($initialCount - $count))

if [ $count -eq 0 ]
then
	echo status green
   exit 0
elif [ $count -lt 0 ]
then   
   echo one or more duplicate processes found
   exit 1
elif [ $count -eq $initialCount ]
then
	echo none of process required is running
	exit 3
elif [ $count -gt 0  ]
then
   echo some of the required processes are not running
   found=$((initialCount-count))
   echo required $initialCount, found $found
   exit 2
else
   exit 0
fi
