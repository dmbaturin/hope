# - delete break between query and response
# - insert hyperlink after queries (not done yet)

BEGIN {
	query_line = 0
	end_query_pending = 0
	verb_prefix = "     "
	nqs = nrs = 0
}

NR == 1 {
	print "% This file was automatically generated from " FILENAME
	print "% (Why not edit that instead?)"
	print ""
}

/^\\begin{(pseudocode|definition|verbatim)}/ {
	flush_query()
	begin_verb()
	next
}

/^\\end{(pseudocode|definition|verbatim)}/ {
	end_verb()
	next
}

/^\\begin{query}/ {
	flush_query()
	in_query = 1
	query_line = FNR
	print "\\begin{latexonly}"
	begin_verb()
	next
}

/^\\end{query}/ {
	end_query_pending = 1
	in_query = 0
	verbatim = 0
	next
}

/^\\begin{response}/ {
	in_response = 1
	if (end_query_pending) {
		end_query_pending = 0
		verbatim = 1
	}
	else
		begin_verb()
	next
}

/^\\end{response}/ {
	in_response = 0
	end_verb()
	next
}

in_query == 1 { q[++nqs] = $0 }
in_response == 1 { r[++nrs] = $0 }

{
	flush_query()
	if (verbatim && length > 0)
		printf "%s", verb_prefix
	print
}

/^\\maketitle/ {
	print "\\begin{htmlonly}"
	print "{\\em Note:}"
	print "The queries in this document may be run by selecting them,"
	print "if your browser can handle forms."
	print "\\end{htmlonly}"
}

function begin_verb() {
	print "\\begin{verbatim}"
	verbatim = 1
}

function end_verb() {
	print "\\end{verbatim}"
	verbatim = 0
}

function flush_query() {
	if (verbatim)
		return
	if (end_query_pending) {
		end_verb()
		end_query_pending = 0
	}
	if (nqs > 0 || nrs > 0) {
		anchor = "<A HREF=\"../try.cgi?file=" FILENAME "&line=" query_line "\">"
		print "\\end{latexonly}"
		print "\\begin{rawhtml}"
		print "<PRE>"
		for (i = 1; i <= nqs; i++) {
			line = quote(q[i])
			match(line, " *")
			print verb_prefix substr(line, 1, RLENGTH) anchor substr(line, RLENGTH+1) "</A>"
		}
		for (i = 1; i <= nrs; i++)
			print verb_prefix quote(r[i])
		print "</PRE>"
		print "\\end{rawhtml}"
		nqs = nrs = 0
	}
}

function quote(s) {
	gsub("&", "\\&amp;", s)
	gsub("<", "\\&lt;", s)
	gsub(">", "\\&gt;", s)
	return s
}
