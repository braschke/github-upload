#ifndef __STRMATCH_SOURCE_INCLUDED
#define __STRMATCH_SOURCE_INCLUDED
/*****************************************************************************/
/* strmatch.c,  stringmatch utilities
 *
 * Kleine Bibliothek mit Pattern Matching Utilities
 *
 * Copyright (c) Dr.-Ing. Carsten Dehning, HIKO GdbR, Febr. 1994
 *
 * Diese Library stellt Routinen zur Verfuegung, um Zeichketten mit
 * UNIX-typischen Wildcards zu bearbeiten
 *
 *
 * die wildcard regeln lauten:
 *
 *    * : alle zeichen, auch keine
 *        * == "", "hallo", "XyZUvW}=="
 *
 *    + : wie '*' aber ein zeichen muss mindestens vorhanden sein
 *        + ==  "hallo", "XyZUvW}=="
 *        + != ""
 *
 *    ? : EIN einziges beliebiges Zeichen, das vorhanden sein muss
 *        ? == "A", "b", "0"
 *        ? != "ab", "", "hallo"
 *
 *    []:  einfaches zeichen muss aus einem satz von zeichen stammen
 *     [aeioub-d]  == "a", "b", "c", "d", "e", "i"
 *     [aeioub-d]  != "f", "r", "t", "ap"
 *     [^aeioub-d] == "f", "r", "t", "ap"
 *     [^aeioub-d] != "a", "b", "c", "d" .....
 *
 * Um Sonderzeichen +*?[]\ zu benutzen, wird diesen Zeichen ein
 * '\' vorangestellt und ihre Funktion damit unwirksam:
 *                   \* ,  \? ,  \[ ,  \] ,  \\
 *
 *
 * Diese Quelle beinhaltet die folgenden Bibliotheksroutinen
 *
 *
 */
#include "stdheader.h"
#include <stdlib.h>

#define C_ANY    TEXT('*')  /* jedes zeichen, auch keines */
#define C_ONE    TEXT('?')  /* exakt ein beliebiges */
#define C_ALO    TEXT('@')  /* AtLeastOne, mindestens ein beliebiges */
#define C_ESC    TEXT('\\') /* umschalter */
#define C_SETB   TEXT('[')  /* SeTBeging, beginn eines zeichensatzes */
#define C_SETE   TEXT('[')  /* SeTEnd, ende eines zeichensatzes */
#define C_SETR   TEXT('[')  /* SeTRange, trenner fuer satzbeginn und satzende */

#if 0
#define C_NOT   '^'
static int NotMatch;
#endif

#define FunctionalChar(_c_) \
       ((_c_) == C_ANY || (_c_) == C_ONE || (_c_) == C_ALO )


/***************************************************************************/
 static int CheckWildcardPattern(const TCHAR *s)
/***************************************************************************/
/*
 * teste, ob der wildcard string 's' unerlaubte zeichensequenzen beinhaltet
 * sonderzeichen sind '\' und '[]' paare
 *
 * return: 0 = okay, sonst fehler
 */
{
   for ( ; *s; s++)
   {
      switch (*s)
      {
         case C_ESC: /* checke escape sequenz */
            if (!*++s) return ESTR_ESC; /* sequenz nicht abgeschlossen */
            break;

     #ifdef C_NOT
         case C_NOT: /* checke negation */
            if (!*++s) return ESTR_NOT; /* sequenz nicht abgeschlossen */
            notmatch = 1;
            break;
     #endif
         case C_SETB :  /* teste ob ein satz von zeichen angegeben wurde */
            s++;                       /* hinter die [ */
            while (*s && *s != C_SETE)  /* schleife bis ] */
            {
               if (*s == C_ESC && !*++s)
                  break;

     #ifdef C_NOT
               if (*s == C_NOT && !*++s)
                  break;
     #endif

               if (*++s == C_SETR)
               {
                  if (!*++s || *s == C_SETE)
                     return ESTR_RANGE;

                  if (*s == C_ESC && !*++s)
                     break;
                  s++;
               }
            }

            if (*s != C_SETE)
               return ESTR_BRACKET;         /* ] fehlt */
            break;

         default:
            break;
      }
   }

   return 0;
}


/***************************************************************************/
 static int CharInSet(const TCHAR *set, TCHAR c)
/***************************************************************************/
/*
 *  zeichen 'c' mit der sequenz 'set' eines zeichensatzes [] auswerten
 *  return:  1 - ein treffer, 0 - kein treffer
 */
{
   int  hit = 1;  /* default: zeichen MUSS im set auftreten */


   /* invertiert suchen? */
   if (*++set == '^') /* C_NOT */
   {
      set++;
      hit = 0; /* zeichen darf NICHT im set auftreten */
   }

   /* schleife bis klammerende oder fehler */
   for (; *set && *set != C_SETE; set++)
   {
      if (*set == C_ESC)
           set++;                   /* escape */

      if (set[1] == C_SETR )           /* bereich a-b testen */
      {
         const TCHAR *h = set + 2;   /* hinter das '-' */

         /* ist dies ein escape? */
         if (*h == C_ESC)
              h++;

         /* liegt das zeichen im bereich? */
         if (*set <= c && c <= *h)
              return hit;
         set = h;
         continue;
      }

      if (*set == c)  /* single TCHARacter */
         return hit;
   }

   return !hit;
}

/***************************************************************************/
 static int CheckMatch (const TCHAR *pat, const TCHAR *s)
/***************************************************************************/
/*
 * teste, ob der string 's' die bedingungen des wildcard-patterns 'p'
 * erfuellt.
 * return:   0=treffer, sonst kein treffer
 */
{
   int anymatch = 0;


   while (*pat && *s)
   {
      switch (*pat)
      {
         case C_ALO:
            if (!*s++) return 1; /* fehler, zeichen fehlt */
            anymatch = 1;
            break;


         case C_ANY:
            anymatch = 1;
            break;


         case C_ONE:
            if (!*s++) return 1; /* fehler, zeichen fehlt */
            /* if (anymatch) anymatch--; */
            break;


         case C_SETB:
            if (anymatch)
            {
               while (*s && !CharInSet(pat,*s))
                  s++;
               anymatch = 0;
            }
            else if (!CharInSet(pat,*s++))
               return 1;

            /* positioniere 'pat' ans ende des satzes */
            while (*pat != C_SETE)
               pat += (*pat == C_ESC) ? 2 : 1;
            break;


         case C_ESC: /* escape */
            pat++;  /* ... und weiter */


         default :
            if (anymatch)
            {
               /* suche, bis string mit pattern uebereinstimmt */
               while (*s && *s != *pat)
                  s++;

               if (!*s++) return 1;
               anymatch = 0;
               break;
            }

            /* muessen exakten treffer haben */
            if (*pat != *s++)
               return 1;

            break;
      }

      pat++;
   }

   while (*pat == C_ANY)
      pat++;

   return (!*pat && (anymatch || !*s))
      ? 0 : 1; /* es fehlt 's' noch an zeichen */
}

/***************************************************************************/


#define MAXPATT  8
static TCHAR *WildPattern[MAXPATT] =
 { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };


/***************************************************************************/
 C_FUNC_PREFIX int strpattern(const TCHAR *s, unsigned pattno)
/***************************************************************************/
/*
 * setze das wildcard pattern 'pattno' fuer die nachfolgende suche
 * mit strmatch(s,pattno)
 */
{
   int code;

   if (pattno >= MAXPATT)
      return ESTR_PATTNO;

   /* haben wir pattern schon zuvor allokiert, dann wieder freigeben */
   if (WildPattern[pattno])
   {
      FREE(WildPattern[pattno]);
      WildPattern[pattno] = NULL;
   }

   /* teste, ob wildcard pattern zulaessig ist */
   if ((code = CheckWildcardPattern(s)) != 0)
      return code;

   /* kopiere den string, da wir ihn uns merken muessen */
   return ((WildPattern[pattno] = STRDUP(s)) != NULL) ? 0 : ESTR_NOMEM;
}

/***************************************************************************/
 C_FUNC_PREFIX int strmatch(const TCHAR *s, unsigned pattno)
/***************************************************************************/
/*
 * teste, ob der string 's' ein treffer fuer das zuvor mit strpattern()
 * definierte wildcardmuster 'pattno' ist.
 *
 * return:   0=treffer, sonst kein treffer oder fehler
 */
{
   return (pattno < MAXPATT && WildPattern[pattno])
     ? CheckMatch(WildPattern[pattno],s) : ESTR_NOWILD;
}

/***************************************************************************/

/* #define TEST */
#ifdef TEST
#include <stdio.h>
#include <stdlib.h>

/***************************************************************************/
 int main(int argc, char *argv[])
/***************************************************************************/
{
   if (argc < 3)
   {
      printf( "Syntax: xxx pattern text\n");
      return 1;
   }

   switch (strpattern(argv[1],7))
   {
      case ESTR_ESC:
         printf("Invalid Escape character\n");
         return 1;

      case ESTR_BRACKET:
         printf("Unlimited range in bracket\n");
         return 1;

      case ESTR_RANGE:
         printf("Unmatched bracket\n");
         return 1;

      case ESTR_NOMEM:
         printf("Can't malloc\n");
         return 1;

      case ESTR_PATTNO:
         printf("Invalid pattern index\n");
         return 1;
   }

   puts(strmatch(argv[2],7) ? "No match" : "Match valid");
   return 0;
}
#endif

/***************************************************************************/
#endif
