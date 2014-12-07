# This gets run on op.h whenever it changes, to produce op.sed

# Extract the values of minprec and maxprec from the header file

$1 == "#define" && $2 == "MINPREC" { minprec = $3 }
$1 == "#define" && $2 == "MAXPREC" { maxprec = $3 }

END {	# Generate a sed script to do the following to the Yacc input:

	# (1)	insert a dummy symbol before the binary operator tokens.
	#	(The preferences are added later by Assoc.sed.)

	print		"/^%token[ \t]*BINARY/i\\"
	print		"%token  BIN_BASE"

	# (2)	group BINARY grammar rules with their actions,
	#	so they can all be replicated together.
	#	Assumes such rules always have actions, that the final '}'
	#	ends a line, and that no '}' within the action ends a line.

	print		"/^.*[:|].*BINARY.*[^}]$/ {"
	print		"	: loop"
	print		"	N"
	print		"	/}$/!b loop"
	print		"}"

	# (3)	replicate all lines containing BINARY with
	#	LBINARY<min>, RBINARY<min>, ..., LBINARY<max>, RBINARY<max>
	#	If a BINARY rule comes first in a production, all copies
	#	after the first have "nonterminal :" changed to "\t|".

	print		"/BINARY/ {"
	print		"	s/BINARY/LBINARY" minprec "/"
	print		"	p"
	print		"	s/^[a-z0-9_]*[ \t]*:/\t|/"
	print		"	s/LBINARY/RBINARY/"
	for (i = minprec+1; i <= maxprec; i++) {
		print	"	p"
		print	"	s/RBINARY" (i-1) "/LBINARY" i "/"
		print	"	p"
		print	"	s/LBINARY/RBINARY/"
	}
	print		"}"
}
