#! /bin/bash
var=0
var1=0

if [ $# -ge 2 ] ; then 
        for(( k=1; k<=$#; k++ )) ; do
	eval h=\$$k
	expr $h + 1 &> /dev/null
	if [[ $? != 0 ]]
	then
                var1=1
		break 	
	fi
	done      
	if [ $var1 -eq 0 ];then
		amount=$[$2-$1+$#-2]
		if [ $1 -lt $2 ] ; then

		if [ $amount -lt 200 ] ; then
			for ((i=0;i<=$[$2-$1];i++))
			do
				dir=$[$1+i]
				if [ -e $dir ] ; then :
				else				
					mkdir $dir
					echo "New directry: $[$1+i] has been created."
					var=1
				fi
			done
		
			if [ "$3" ] ; then 
				for ((j=3;j<=$#;j++))
				do
					eval dir=\$$j
					if [ -e $dir ] ; then :				
					else 
						mkdir $dir
						echo "New directry: $dir has been created."
						var=1
					fi
				done
			fi
		
			if [ $var -eq 0 ] ; then
			echo	"所有目录均已存在，无需创建"
			fi	
		
		else
			echo "The number of directory should less than 200" 
		fi

		else
			echo "The 1st number should less than 2nd number."
		fi
        else  
                 echo "The paraments should all be number"
	fi
else
	echo 'The number of paraments is less than 2'
fi

