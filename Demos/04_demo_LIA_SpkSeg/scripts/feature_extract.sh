#!/bin/bash

if [ $# != "4" ]; then
echo "usage: $0 liste <type Param> <rep> <sph ext>"
exit 1
fi

SPRO_BIN_DIR="./scripts/"
SPH_DIR="./data/sph/"
PRM_DIR="./data/$3/"
PRM_EXT="$4.prm"
SPH_EXT="$4.sph"
echo $SPRO_BIN_DIR

if [ "$2" == "21" ]; then
	OPTS="-F sphere -p 20 -e -k 0"
fi
if [ "$2" == "19" ]; then
	OPTS="-F sphere -p 19 -k 0 -l 30.0"
fi
if [ "$2" == "39" ]; then
	OPTS="-F sphere -p 12 -e -D -A -k 0"
fi
if [ "$2" == "51" ]; then
	OPTS="-F sphere -p 16 -e -D -A -k 0"
fi
if [ "$2" == "34" ]; then
	OPTS="-F sphere -p 16 -e -D -k 0"
fi
if [ "$2" == "3000" ]; then
	OPTS="-F sphere -p 20 -e -k 0 -i 0 -u 3000"
fi
if [ "$2" == "60" ]; then
	OPTS="-F sphere -p 19 -e -D -A -k 0"
fi


for i in `cat $1`;do
	
	echo ${SPRO_BIN_DIR}."/sfbcep" ${OPTS} ${SPH_DIR}/${i}${SPH_EXT}  $PRM_DIR/$i$PRM_EXT
	$SPRO_BIN_DIR/sfbcep $OPTS $SPH_DIR/$i$SPH_EXT $PRM_DIR/$i$PRM_EXT
done
