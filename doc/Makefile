# Generated automatically from Makefile.in by configure.
SHELL	= /bin/sh

prefix	= /usr/local
AWK	= mawk
INSTALL	= /usr/bin/install -c
docdir	= ${prefix}/share/doc/hope
srcdir	= ${prefix}/share/doc/hope/src

htmldir	= $(docdir)

tex_srcs = ref_man.tex
src_srcs = hope_tut.src examples.src
Roger	= Roger Bailey &lt;rb@doc.ic.ac.uk&gt;
Ross	= Ross Paterson &lt;ross@soi.city.ac.uk&gt;

made_tex = op.tex Standard.tex

# It is also possible to generate HTML versions: you'll need Latex2html,
# c.f. http://cbl.leeds.ac.uk/nikos/tex2html/doc/latex2html/latex2html.html
L2H_FLAGS = -info '' -no_navigation # -dont_include latexonly
LATEX2HTML = latex2html $(L2H_FLAGS)

tex_made = $(src_srcs:.src=.tex)
tex_files = $(tex_srcs) $(tex_made)
dirs	= $(tex_files:%.tex=%)
html	= $(tex_files:%.tex=%/index.html)
docs	= $(tex_files:%.tex=%.dvi) $(tex_files:%.tex=%.ps)

.SUFFIXES:
.SUFFIXES: .ps .dvi .tex .src

%.tex: %.src
	$(AWK) -f verb.nawk $*.src >$@

%.dvi: %.tex
	latex $*
	if grep -s '^LaTeX Warning: Citation .* undefined' $*.log;\
	then	bibtex $*; latex $*;\
	fi
	if grep -s '^LaTeX Warning: Label(s) may have changed' $*.log;\
	then	latex $*;\
	fi

%.ps: %.dvi
	dvips $* -o $@

docs: $(docs)

html: $(html)

all: docs html

ref_man.dvi: $(made_tex) hope.bib

# Latex files derived from C and Hope sources

op.tex: ../src/op.h
	sed -n 's/^#define[ 	][ 	]*\(M[IA][NX]PREC\)[ 	][ 	]*\([0-9]*\).*/\\newcommand{\\\1}{\2}/p' ../src/op.h >$@

../src/op.h:
	cd ../src; make op.h

# can't use echo here as BSD and SYSV versions treat backslashes differently.
Standard.tex: ../lib/Standard.hop h2l.awk
	../sh/header ../lib/Standard.hop | $(AWK) -f h2l.awk | expand >$@

# HTML versions of the documents

hope_tut/index.html: hope_tut.tex hope_tut.dvi
	rm -rf hope_tut
	$(LATEX2HTML) -address '$(Roger)' hope_tut.tex

ref_man/index.html: ref_man.dvi
	rm -rf ref_man
	$(LATEX2HTML) -address '$(Ross)' ref_man.tex

examples/index.html: examples.tex examples.dvi
	rm -rf examples
	$(LATEX2HTML) -address '$(Ross)' examples.tex

install: all
	$(INSTALL) -d $(docdir)
	for file in $(docs); do gzip <$$file >$(docdir)/$$file.gz; done
	$(INSTALL) -d $(htmldir)
	for dir in $(dirs); do rm -rf $(htmldir)/$$dir; done
	cp -r $(dirs) $(htmldir)
	# cp $(src_srcs) $(srcdir)

clean:
	rm -f *.blg *.dvi *.log *.ps *.pdf
	rm -f $(made_tex)
	rm -rf $(dirs)

distclean:
	rm -f *.aux *.bbl *.glo *.idx *.lof *.lot *.toc

clobber: clean
	rm -f *.aux *.bbl *.glo *.idx *.lof *.lot *.toc
	rm -f $(tex_made)
