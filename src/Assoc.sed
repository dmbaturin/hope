/^%token[ 	][ 	]*[LRN]BINARY[0-9][0-9]*$/ {
	/LBINARY/ s/token/left/
	/RBINARY/ s/token/right/
	/NBINARY/ s/token/nonassoc/
}
