#Look at the Processlist.txt file and chekc if all the required processes are
#running. The first line in ProcessList.txt specifies the number of processes to
#look for.

#The variable 'LISTFILE' holds the filename of the ProcessList.

#exit status
#0-Status Green (all required processes found and running)
#1-One or more duplicate processes found
#2-Some of the required processes not running
#3-Cannot open the given file name in the LISTFILE variable

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
elif [ $count -gt 0  ]
then
   echo some of the required processes are not running
   found=$((initialCount-count))
   echo required $initialCount, found $found
   exit 2
fi
