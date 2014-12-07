BEGIN {
	# modes
	start = 0
	code = 1
	text = 2
	# initial state
	mode = start
	blank = 0
	code_prefix = "    "
}

/^$/ {
	blank = 1
}

/^! / {
	if (mode == code)
		print "\\end{verbatim}"
	else if (blank)
		print ""
	blank = 0
	mode = text
	print substr($0, 3)
}

/^!! / {
	if (mode == code)
		print "\\end{verbatim}"
	blank = 0
	mode = text
	print "\\subsection{" substr($0, 4) "}"
}

/^!	/ {
	if (mode != code)
		print "\\begin{verbatim}"
	else if (blank)
		print ""
	blank = 0
	mode = code
	print code_prefix "!        " substr($0, 3)
}

/^[^!]/ {
	if (mode != code)
		print "\\begin{verbatim}"
	else if (blank)
		print ""
	blank = 0
	mode = code
	print code_prefix $0
}

END {
	if (mode == code)
		print "\\end{verbatim}"
}
