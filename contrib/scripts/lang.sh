#!/bin/bash

LANG=${1:-de}
APPNAME=${2:-caveexpress}

grep -PohR 'tr\("\K[^"]*' src/modules src/${APPNAME} | sort | uniq | awk -v "app=${APPNAME}" -v "lang=${LANG}" '
BEGIN {
	old_FS = FS
	FS     = "\\|"
	status = 0
	file   = "base/" app "/lang/" lang ".lang"
	source = ""

	while (getline) {
		msgid = $1
		exists = 0
		input  = "cat " file
		t      = ""
		while (input | getline t) {
			split(t, msg, "\\|")
			if ( msg[1] != msgid ) {
				continue
			}
			if ( msg[2] != "" ) {
				print msg[1] "|" msg[2]
				exists = 1
			}
			break
		}
		close(input)
		if ( exists == 1 )
			continue

		print $1"|"$1
	}

	FS = old_FS

	exit status
}' > base/${APPNAME}/lang/${LANG}.lang.tmp
mv base/${APPNAME}/lang/${LANG}.lang.tmp base/${APPNAME}/lang/${LANG}.lang
