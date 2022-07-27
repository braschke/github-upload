#pragma once
#ifndef isshmeta_SOURCE_INCLUDED
#define isshmeta_SOURCE_INCLUDED
/* isshmeta.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Check if char 'c' is a bourne shell or MSWin cmd meta character.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2005/Sep/24: Carsten Dehning, Initial release
 *    $Id: isshmeta.c 4327 2016-05-06 17:36:22Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int isshmeta(int c)
{
   typedef struct _META_MAP_TABLE
   {
      int c; /* char */
      int t; /* type */
   } META_MAP_TABLE;

   static META_MAP_TABLE mmt[] =
   {
      { '|' ,  ISSHMETA_TERM   },
      { '&' ,  ISSHMETA_TERM   },
      { '>' ,  ISSHMETA_TERM   },
      { '<' ,  ISSHMETA_TERM   },
      { ';' ,  ISSHMETA_TERM   },

      { '"' ,  ISSHMETA_QUOTE  },
      { '\'',  ISSHMETA_QUOTE  },
      { '´' ,  ISSHMETA_QUOTE  },
      { '`' ,  ISSHMETA_QUOTE  },

      { '*' ,  ISSHMETA_WILD   },
      { '?' ,  ISSHMETA_WILD   },

      { '#' ,  ISSHMETA_COMT   },

      { '(' ,  ISSHMETA_PAREN  },
      { ')' ,  ISSHMETA_PAREN  },
      { '[' ,  ISSHMETA_PAREN  },
      { ']' ,  ISSHMETA_PAREN  },
      { '{' ,  ISSHMETA_PAREN  },
      { '}' ,  ISSHMETA_PAREN  },
      { '~' ,  ISSHMETA_PAREN  },

      { '!' ,  ISSHMETA_ANY    },
      { '$' ,  ISSHMETA_ANY    },
      { '\\',  ISSHMETA_ANY    },
      {  0  ,  ISSHMETA_NONE   }
   };

   if (c)
   {
      const META_MAP_TABLE *m;

      for(m=mmt; m->c; m++)
         if (m->c == c) return m->t;
   }
   return ISSHMETA_NONE;
}
#endif
