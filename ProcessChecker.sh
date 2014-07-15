#Look at the Processlist.txt file and chekc if all the required processes are
#running. The first line in ProcessList.txt specifies the number of processes to
#look for.

#The variable 'values' holds the filename of the ProcessList.

#exit status
#0-Status Green (all required processes found and running)
#1-One or more duplicate processes found
#2-Some of the required processes not running
#3-Cannot open the given file name in the values variable

#use 'echo $?' command after the program terminates to find the status

count=0
values="ProcessList.txt"
firstLine=0
initialCount=0

if ! [ -f $values  ]
then
   echo "file $values do not exist"
   exit 3
fi

while read LINE
do
   echo "$LINE" | grep -q "^#.*"
   if [ $? -eq 0 ]
   then
      continue
   fi
   if [ $firstLine -eq 0  ]
   then
      count=${LINE%#*}
      initialCount=$count
      firstLine=1
      continue
   else
		b=${LINE%#*}
      b=${b%[*}
      if [ -z "$b" ]
      then
         continue
      fi
		PCOUNT=$(ps aux | grep "$b" | grep -v grep | wc -l)
		if [ $PCOUNT -ne 0 ]
      then
         c=$(ps aux | grep "$b" | grep -v grep | awk ' { print $2 } ')
         echo $PCOUNT process found with the name $b with PID $c
			count=$((count-PCOUNT))
      else
			echo no process found with the name $b
		fi
	fi
done < "$values"

if [ $count -eq 0 ];then
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
