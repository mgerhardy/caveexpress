#!/bin/bash

#set -x
set -e

SPRITE=
SRC=middle
TARGET=back

usage() {
	echo "Usage: $0 [--help|-h] [--source|-sl <layer>] [--target|-tl <layer>] [--sprite <file>]"
	echo " --help -h         - Shows this help message"
	echo " --source -s       - The source layer (front, middle, back) - default: middle"
	echo " --target -t       - The target layer (front, middle, back) - default: back"
	echo " --sprite          - The sprite base name to convert - example: npc-grandpa-walk-left-"
	exit 1
}

error() {
	echo "Error: $@"
	exit 1
}

while [ $# -gt 0 ]; do
	case "$1" in
		--sprite)
			SPRITE=$2
			shift
			shift
			;;
		--target|-t)
			TARGET=$2
			shift
			shift
			;;
		--source|-s)
			SRC=$2
			shift
			shift
			;;
		--help|-h|*)
			usage
			;;
		-*)
			error "invalid option $1"
			;;

	esac
done

[ -z "$SPRITE" ] && error "No sprite given"

echo "move ${SPRITE} from ${SRC} to ${TARGET}"
cd contrib/assets/png

for i in ${SPRITE}*${SRC}*.png; do
	VAR1=$(echo $i | sed "s/${SRC}/${TARGET}/g")
	echo "move $i => $VAR1"
	mv $i $VAR1;
done

cd ..
cd png-packed

for i in *.lua *.tps; do
	echo "replace in $i"
	sed -i "s/\(.*${SPRITE}.*\)${SRC}-\([0123456789][0123456789].*\)/\1${TARGET}-\2/g" $i
done
