/* copyright <happyteam@thinkware.co.kr> */
#ifndef _HWDEFINE_H__
#define _HWDEFINE_H__

#include "thstring.h"
#include "s_errormsg.h"


static thlib::thString g_pnm_happy    = "happyway";
static thlib::thString g_pnm_routeagent = "routeagent";
static thlib::thString g_pnm_rdbctrl  = "rdbctrl";
static thlib::thString g_pnm_fake    = "fake";


/* average ratio on korea (unit:meter/0.01sec) */
#define HYRATIO              (0.30879f)
#define HXRATIO              (0.27654f)

/**
 *  Phygical & Logical Rank Control Information
 */
///  Rank count
#define RANK_COUNT            (3)
#define RANK_UPCOUNT          (2)

///  rank0, 1 map size
#define RANK0_MAPWIDTH          (180000)
#define RANK0_MAPHEIGHT          (120000)
#define RANK1_MAPWIDTH          (90000)
#define RANK1_MAPHEIGHT          (60000)

#endif

