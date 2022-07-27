#pragma once
#ifndef stdheader_HEADER_INCLUDED
#define stdheader_HEADER_INCLUDED
/* stdheader.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Avoid reading standard larger header files more than once. (#pragma once)
 *    Must be the first ever #included before all other included including std c headers.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: stdheader.h 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdconfig.h"
#include "realtype.h"

/*
 * These guys are required everywhere
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "stdcasts.h"
#include "stdmacros.h"
#include "stdprotos.h"

#endif
