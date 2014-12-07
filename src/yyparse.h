/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TYPEVAR = 258,
     ABSTYPE = 259,
     DATA = 260,
     TYPESYM = 261,
     DEC = 262,
     INFIX = 263,
     INFIXR = 264,
     USES = 265,
     PRIVATE = 266,
     DISPLAY = 267,
     SAVE = 268,
     WRITE = 269,
     TO = 270,
     EXIT = 271,
     EDIT = 272,
     DEFEQ = 273,
     OR = 274,
     VALOF = 275,
     IS = 276,
     GIVES = 277,
     THEN = 278,
     FORALL = 279,
     MODSYM = 280,
     PUBCONST = 281,
     PUBFUN = 282,
     PUBTYPE = 283,
     END = 284,
     MU = 285,
     IN = 286,
     WHEREREC = 287,
     WHERE = 288,
     ELSE = 289,
     BIN_BASE = 290,
     LBINARY1 = 291,
     RBINARY1 = 292,
     LBINARY2 = 293,
     RBINARY2 = 294,
     LBINARY3 = 295,
     RBINARY3 = 296,
     LBINARY4 = 297,
     RBINARY4 = 298,
     LBINARY5 = 299,
     RBINARY5 = 300,
     LBINARY6 = 301,
     RBINARY6 = 302,
     LBINARY7 = 303,
     RBINARY7 = 304,
     LBINARY8 = 305,
     RBINARY8 = 306,
     LBINARY9 = 307,
     RBINARY9 = 308,
     NONOP = 309,
     LAMBDA = 310,
     IF = 311,
     LETREC = 312,
     LET = 313,
     CHAR = 314,
     LITERAL = 315,
     NUMBER = 316,
     IDENT = 317,
     APPLY = 318,
     ALWAYS_REDUCE = 319
   };
#endif
#define TYPEVAR 258
#define ABSTYPE 259
#define DATA 260
#define TYPESYM 261
#define DEC 262
#define INFIX 263
#define INFIXR 264
#define USES 265
#define PRIVATE 266
#define DISPLAY 267
#define SAVE 268
#define WRITE 269
#define TO 270
#define EXIT 271
#define EDIT 272
#define DEFEQ 273
#define OR 274
#define VALOF 275
#define IS 276
#define GIVES 277
#define THEN 278
#define FORALL 279
#define MODSYM 280
#define PUBCONST 281
#define PUBFUN 282
#define PUBTYPE 283
#define END 284
#define MU 285
#define IN 286
#define WHEREREC 287
#define WHERE 288
#define ELSE 289
#define BIN_BASE 290
#define LBINARY1 291
#define RBINARY1 292
#define LBINARY2 293
#define RBINARY2 294
#define LBINARY3 295
#define RBINARY3 296
#define LBINARY4 297
#define RBINARY4 298
#define LBINARY5 299
#define RBINARY5 300
#define LBINARY6 301
#define RBINARY6 302
#define LBINARY7 303
#define RBINARY7 304
#define LBINARY8 305
#define RBINARY8 306
#define LBINARY9 307
#define RBINARY9 308
#define NONOP 309
#define LAMBDA 310
#define IF 311
#define LETREC 312
#define LET 313
#define CHAR 314
#define LITERAL 315
#define NUMBER 316
#define IDENT 317
#define APPLY 318
#define ALWAYS_REDUCE 319




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 101 "y.tab.y"
typedef union YYSTYPE {
	Num	numval;
	int	intval;
	Text	*textval;
	String	strval;
	Natural	charval;
	Type	*type;
	TypeList *typelist;
	DefType	*deftype;
	QType	*qtype;
	Expr	*expr;
	Branch	*branch;
	Cons	*cons;
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 180 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



