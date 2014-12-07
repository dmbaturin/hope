# - delete break between query and response
# - insert hyperlink after queries (not done yet)

BEGIN {
	query_line = 0
	end_query_pending = 0
	verb_prefix = "     "
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

{
	flush_query()
	if (verbatim && length > 0)
		printf "%s", verb_prefix
	print
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
}
