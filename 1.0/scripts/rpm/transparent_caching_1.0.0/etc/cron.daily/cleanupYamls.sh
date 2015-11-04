####################################################
# Script to cleanup old TCS config.yaml files that #
# are saved when the config.yaml file is updated   #
# through the mgt sys or api_server api.           #
####################################################

#!/bin/sh

FILES=/etc/sysconfig/transparent_caching/yamls/config.*
FILECOUNT=$(ls $FILES | wc -l)
KEEPFILECOUNT=20
DELETECOUNT=`expr $FILECOUNT - $KEEPFILECOUNT`
n=1


if [ "$DELETECOUNT" -lt 1 ]; then
 echo "No backup config.yaml files to delete"
 exit
else
 echo "Removing $DELETECOUNT backup config.yaml files"
fi

for f in `ls -rt $FILES`; do
 if [ "$n" -gt "$DELETECOUNT" ]; then
  exit 
 fi
 rm -f $f
# echo Deleted file: $f
 n=`expr $n + 1`
done

exit 0
