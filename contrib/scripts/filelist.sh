#!/bin/bash

set -e
#set -x

if [ $# -eq 0 ]; then
	echo "Usage: $0 filename basedir [basedirs]"
	exit 1
fi

FILENAME=$1
shift
BASEDIR=$1
shift
while [ $# -gt 0 ]; do
	i=$1
	echo "==> ${i}";
	echo "if (basedir == \"${i}/\") {" >> $FILENAME;
	for file in ${BASEDIR}/${i}/*; do
		echo "entriesAll.push_back(\"`basename ${file}`\");" >> $FILENAME;
	done;
	echo "return entriesAll;" >> $FILENAME;
	echo "}" >> $FILENAME;
	shift
done
