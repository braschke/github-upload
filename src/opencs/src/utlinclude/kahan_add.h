#ifndef kahan_add_HEADER_INCLUDED
#define kahan_add_HEADER_INCLUDED
/* kahan_add.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Kahan addition with summation error correction.
 *    This CPP macro is used with the summation of multible floating point values.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2013/Mai/22: Carsten Dehning, Initial release
 *    $Id: kahan_add.h 3471 2015-09-30 17:31:01Z dehning $
 *
 *****************************************************************************************
 */
#ifndef USE_KAHAN_ADD
   #define USE_KAHAN_ADD   0
#endif

#if USE_KAHAN_ADD

   #define KAHAN_ADD_DOUBLE(_sum,_dval,_loss)\
   {\
      const double _tmp_val = _dval - _loss;\
      const double _tmp_sum = _sum + _tmp_val;\
      _loss = (_tmp_sum - _sum) - _tmp_val;\
      _sum  = _tmp_sum;\
   }

   #define KAHAN_ADD_FLOAT(_sum,_fval,_loss)\
   {\
      const double _tmp_val = CAST_DOUBLE(_fval) - _loss;\
      const double _tmp_sum = _sum + _tmp_val;\
      _loss = (_tmp_sum - _sum) - _tmp_val;\
      _sum  = _tmp_sum;\
   }

#endif

#endif
