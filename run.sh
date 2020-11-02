#!/bin/bash



make all

#NN=$(grep NSENS config.txt >> /dev/null | awk -F" " '{print $2}')

NN=$(grep NSOCKETS config.txt  | awk -F" " '{print $2}')

#echo " = = = = =  B E F O R E  server = = = =  NSENS=$NN"


./serverFstd &

echo " = = = = =  A F T E R server = = = =  NSOCKETS=$NN"

for i in `seq 1 1 $NN`; do
sleep 1
./client &
done


sleep 10

killall serverFstd
killall client



exit 0
