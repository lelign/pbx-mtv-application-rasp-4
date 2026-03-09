# bin bash!
counter=1
for f in $(find ../508_source_soft_exp -type f); do
#echo $f
#echo $(basename $f)
file_local=$(find ./ -type f -name $(basename $f) )
unset check_diff
if [[ -f $f && -f $file_local ]]; then
	check_diff=$(diff $f $file_local)
fi
if [[ -n $check_diff ]]; then
	echo $counter
	echo $file_local
	diff $f $file_local
	cp $f $file_local
	let counter=$counter+1
	#echo $check_diff
	echo " "
fi
done






#for f in $(find ../508_source_soft_exp -type f); do echo $f; done
