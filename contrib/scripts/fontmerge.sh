#!/bin/bash

# fontbuilder lua table export

#set -e
#set -x

FILE=${1:-font-}
FILEDIR=$(dirname $(readlink -f $FILE))
FILE=$(basename $(readlink -f $FILE))
DIR=$(dirname $(readlink -f $0))
ROOT=$DIR/../..

# filenames can be something like font_family_8.lua and font_family_20.lua
# these are renamed to font-8 and font-20 (those are the names we expect in
# the lua font files)

if [ "$FILE" != "font-" ]; then
	# also rename png files
	rename "s/$FILE/font-/g" $FILEDIR/$FILE*
fi
rename "s/PNG$/png/g" $FILEDIR/$FILE*
sed -i "s/$FILE/font-/g" $FILEDIR/font-*.lua
sed -i "s/.png//g" $FILEDIR/font-*.lua
for game in caveexpress cavepacker; do
	for i in $(ls -1 $FILEDIR/$FILE*.png 2> /dev/null); do
		echo "$i => $FILEDIR/$FILE*.png"
#		cp $i $ROOT/contrib/assets/png/$game/ui
	done
done

for game in caveexpress cavepacker; do
	FONTSFILE=$ROOT/base/$game/fonts.lua
	echo "-- data is exported via fontbuilder" > $FONTSFILE
	echo "" >> $FONTSFILE
	echo "fonts = {" >> $FONTSFILE
	for i in $FILEDIR/font-*.lua; do
		file=$(basename $i)
		echo "	[\"${file%.lua}\"] = {" >> $FONTSFILE
		sed 's/\(.*\)/\t\t\1/g' $i >> $FONTSFILE
		echo "	}," >> $FONTSFILE
	done
	sed -i 's/\([^{,]\)$/\1,/g' $FONTSFILE
	echo "}" >> $FONTSFILE
done

