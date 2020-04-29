#ifndef BISON_GRAM_H
# define BISON_GRAM_H

#ifndef YYSTYPE
typedef union {
  char* symbol;
  double number;
  char* string;
  } yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	SYMBOL	257
# define	STRSYMBOL	258
# define	NUMBER	259
# define	STRING	260
# define	T_ENDOFFILE	261
# define	T_SEPARATOR	262
# define	T_MOD	263
# define	T_NOT	264
# define	T_OR	265
# define	T_AND	266
# define	T_XOR	267
# define	T_EQ	268
# define	T_NE	269
# define	T_LT	270
# define	T_LE	271
# define	T_GT	272
# define	T_GE	273
# define	T_IF	274
# define	T_THEN	275
# define	T_ELSE	276
# define	T_ENDIF	277
# define	T_LET	278
# define	T_FOR	279
# define	T_TO	280
# define	T_STEP	281
# define	T_NEXT	282
# define	T_WHILE	283
# define	T_WEND	284
# define	T_PRINT	285
# define	T_SUB	286
# define	T_END	287
# define	T_RANDOMIZE	288
# define	UBITNOT	289
# define	UNOT	290
# define	UMINUS	291


extern YYSTYPE yylval;

#endif /* not BISON_GRAM_H */
