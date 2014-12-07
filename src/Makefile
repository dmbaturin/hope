# Generated automatically from Makefile.in by configure.
SHELL	= /bin/sh

# stuff from configure:
prefix = /usr/local
exec_prefix = ${prefix}

# Fiddlable parameters:
# What the program is called.
name	= hope
# Where to put the executable version.
bindir	= ${exec_prefix}/bin
# Small test suite, used by "make check".
testdir = ../test

# more stuff from configure:
AWK	= mawk
CC	= gcc
CFLAGS	= -g -O2 -pipe -pedantic -Wall -W -Wshadow -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wpointer-arith -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations
INSTALL	= /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_PROGRAM = ${INSTALL}
LDFLAGS	= 
LIBS	= -lm 
YACC	= bison -y
mandir	= ${prefix}/man/man1

# Where the standard modules live.
hopelib	= ${prefix}/share/hope/lib

c_srcs = bad_rectype.c builtin.c cases.c char.c char_array.c compare.c \
        compile.c deftype.c eval.c expr.c functor_type.c functors.c \
        interpret.c interrupt.c main.c memory.c module.c newstring.c \
        number.c output.c path.c polarity.c pr_expr.c pr_ty_value.c \
        pr_type.c pr_value.c remember_type.c runtime.c set.c source.c \
        stream.c table.c type_check.c type_value.c value.c yylex.c
parser	= yyparse

sources = $(c_srcs) $(parser).y
c_made	= $(parser).c
h_made	= hopelib.h $(parser).h
other_made = op.sed
tmps	= y.output y.tab.y y.tab.h y.tab.c y.tab.o

assoc	= Assoc.sed
mult_op	= Mult-op.awk

cfiles	= $(c_srcs) $(c_made)
objects	= $(cfiles:.c=.o)

$(name): $(objects)
	$(CC) $(LDFLAGS) -o $@ $(objects) $(LIBS)

all: $(name) $(name).1

install: check $(name).1
	$(INSTALL) -d $(hopelib)
	$(INSTALL_PROGRAM) -s $(name) $(bindir)
	$(INSTALL_DATA) $(name).1 $(mandir)

$(name).1: $(name).1.in
	sed -e 's:@hopelib@:$(hopelib):' -e 's:@fullpath@:$(bindir)/$(name):' $(name).1.in >$@

cfiles: $(c_made) $(h_made)

# Miscellaneous derived files

tags:	$(sources)
	ctags -tw $(sources)

# Various levels of safe clean-up:
#	distclean - temporary files, object files and executables
#	new	- generated files specific to an architecture
#	clean	- all generated files except the main program
#	clobber	- all generated files

distclean: cfiles
	rm -f *.o core a.out errors
	rm -f $(name)
	rm -f $(tmps)

new:
	rm -f *.o core a.out errors $(h_made)
	rm -f $(name)

clean:
	rm -f *.o core a.out errors tags LOG
	rm -f $(h_made) $(c_made) $(other_made)
	rm -f $(tmps)

clobber: clean
	rm -f $(name) $(name).1

# Test a new version of the interpreter, by
# (1) running it on some examples and comparing with the expected output.
# (2) checking all the system modules go through OK.
# If all is well, there will be no output.

errors:	$(name) $(testdir)/*.in $(testdir)/*.out ../lib/*
	for file in $(testdir)/*.in;\
	do	STEM=`basename $$file .in`;\
		HOPEPATH=../lib nice ./$(name) -f $$file 2>&1 |\
			diff - $(testdir)/$$STEM.out |\
			sed "s/^/$$STEM: /";\
	done >$@
	LC_ALL; for file in ../lib/[a-z]*.hop;\
	do	HOPEPATH=../lib nice ./$(name) -f $$file 2>&1;\
	done >>$@

check:	errors
	test ! -s errors

hopelib.h:
	echo '#define HOPELIB "$(hopelib)"' >$@

# Generate parser from $(parser).y by using one sed file (generated from op.h)
# to replicate all lines mentioning BINARY, and another to give the generated
# tokens the correct associativity.

# The hiding of yyerrstatus is a hack for Bison: see yyparse.y

$(parser).h $(parser).c: $(parser).y op.sed $(assoc)
	sed -f op.sed $(parser).y | sed -f $(assoc) >y.tab.y
	$(YACC) $(YFLAGS) -d y.tab.y
	grep -v '^# *line' y.tab.c |\
		sed -e '1s/*malloc(), //' -e 's:^  int yyerrstatus;:/* & */:' >$(parser).c
	mv y.tab.h $(parser).h
	rm -f y.tab.y y.tab.c

op.sed: op.h $(mult_op)
	$(AWK) -f $(mult_op) op.h >$@

# for grammar debugging

y.output: $(parser).y op.sed $(assoc)
	sed -f op.sed $(parser).y | sed -f $(assoc) >y.tab.y
	$(YACC) -v y.tab.y
	rm -f y.tab.y y.tab.c

# Generate dependencies of source files on header files.
# Only inclusions of relative file names yield dependencies.

depend:	cfiles
	../sh/makedepend -- $(DEFS) -- $(c_srcs) $(c_made)

# DO NOT DELETE THIS LINE -- make depend depends on it.
bad_rectype.o: bad_rectype.h config.h cons.h defs.h deftype.h error.h \
	newstring.h structs.h table.h typevar.h
builtin.o: builtin.h cases.h char.h config.h cons.h defs.h deftype.h error.h \
	expr.h heap.h interpret.h newstring.h num.h output.h path.h stream.h \
	structs.h table.h typevar.h value.h
cases.o: cases.h char.h char_array.h config.h defs.h error.h expr.h memory.h \
	newstring.h num.h path.h structs.h table.h
char_array.o: char.h char_array.h config.h defs.h error.h memory.h structs.h
compare.o: cases.h char.h compare.h config.h cons.h defs.h error.h expr.h \
	heap.h newstring.h num.h path.h structs.h table.h value.h
compile.o: cases.h char.h char_array.h compile.h config.h cons.h defs.h \
	error.h expr.h newstring.h num.h path.h structs.h table.h
deftype.o: bad_rectype.h char.h config.h cons.h defs.h deftype.h error.h \
	expr.h functors.h memory.h newstring.h num.h path.h polarity.h \
	structs.h table.h type_check.h typevar.h
eval.o: char.h compile.h config.h defs.h error.h eval.h exceptions.h expr.h \
	interpret.h newstring.h num.h number.h output.h path.h stream.h \
	structs.h table.h type_check.h
expr.o: cases.h char.h compile.h config.h cons.h defs.h error.h expr.h \
	memory.h newstring.h num.h number.h path.h structs.h table.h \
	type_check.h
functor_type.o: char.h config.h defs.h deftype.h error.h functor_type.h \
	heap.h newstring.h num.h path.h structs.h table.h type_value.h \
	typevar.h
functors.o: char.h config.h cons.h defs.h deftype.h error.h expr.h functors.h \
	newstring.h num.h path.h structs.h table.h typevar.h
interpret.o: cases.h char.h char_array.h config.h cons.h defs.h error.h \
	expr.h heap.h interpret.h interrupt.h newstring.h num.h output.h \
	path.h pr_value.h stack.h stream.h structs.h table.h value.h
interrupt.o: config.h defs.h error.h interrupt.h structs.h
main.o: config.h defs.h error.h memory.h module.h newstring.h plan9args.h \
	source.h structs.h
memory.o: align.h config.h defs.h error.h memory.h structs.h
module.o: builtin.h char.h compare.h config.h cons.h defs.h deftype.h error.h \
	expr.h hopelib.h memory.h module.h names.h newstring.h num.h op.h \
	output.h path.h pr_expr.h pr_type.h remember_type.h set.h source.h \
	structs.h table.h typevar.h
newstring.o: align.h config.h defs.h error.h memory.h newstring.h structs.h
number.o: char.h config.h cons.h defs.h error.h expr.h newstring.h num.h \
	number.h path.h pr_expr.h structs.h table.h
output.o: cases.h char.h config.h defs.h error.h expr.h heap.h memory.h \
	newstring.h num.h output.h path.h pr_ty_value.h pr_value.h structs.h \
	table.h type_check.h type_value.h value.h
path.o: config.h defs.h error.h memory.h path.h structs.h
polarity.o: config.h defs.h deftype.h error.h names.h newstring.h polarity.h \
	structs.h table.h typevar.h
pr_expr.o: char.h config.h cons.h defs.h error.h expr.h names.h newstring.h \
	num.h op.h path.h pr_expr.h pr_value.h print.h structs.h table.h
pr_ty_value.o: char.h config.h defs.h deftype.h error.h heap.h names.h \
	newstring.h num.h op.h path.h pr_ty_value.h print.h structs.h table.h \
	type_value.h typevar.h
pr_type.o: config.h cons.h defs.h deftype.h error.h names.h newstring.h op.h \
	polarity.h pr_type.h print.h structs.h table.h typevar.h
pr_value.o: char.h config.h cons.h defs.h error.h expr.h heap.h interpret.h \
	names.h newstring.h num.h op.h path.h pr_expr.h pr_value.h print.h \
	stack.h structs.h table.h value.h
remember_type.o: char.h config.h cons.h defs.h deftype.h error.h expr.h \
	newstring.h num.h path.h remember_type.h structs.h table.h typevar.h
runtime.o: char.h config.h defs.h error.h heap.h memory.h num.h path.h \
	stack.h structs.h type_check.h
set.o: config.h defs.h error.h set.h structs.h
source.o: config.h defs.h error.h exceptions.h interrupt.h module.h \
	newstring.h source.h structs.h
stream.o: builtin.h char.h config.h cons.h defs.h error.h expr.h heap.h \
	newstring.h num.h path.h stream.h structs.h table.h value.h
table.o: config.h defs.h error.h newstring.h structs.h table.h
type_check.o: char.h config.h cons.h defs.h deftype.h error.h exceptions.h \
	expr.h functor_type.h heap.h newstring.h num.h op.h path.h pr_expr.h \
	pr_ty_value.h pr_type.h structs.h table.h type_check.h type_value.h \
	typevar.h
type_value.o: char.h config.h defs.h deftype.h error.h heap.h newstring.h \
	num.h path.h structs.h table.h type_check.h type_value.h typevar.h
value.o: char.h config.h defs.h error.h heap.h num.h path.h structs.h value.h
yylex.o: char.h config.h defs.h error.h names.h newstring.h num.h op.h \
	source.h structs.h table.h text.h typevar.h yyparse.h
yyparse.o: char.h config.h cons.h defs.h deftype.h error.h eval.h expr.h \
	memory.h module.h newstring.h num.h op.h path.h structs.h table.h \
	text.h typevar.h
