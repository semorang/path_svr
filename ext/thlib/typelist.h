/* Copyright 2011 <happyteam@thinkwaresys.com> */
////////////////////////////////
// Generated header: Typelist.h
// Forwards to the appropriate code
// that works on the detected compiler
// Generated on Mon Sep 30 23:14:48 2002
////////////////////////////////

#   if (_MSC_VER >= 1200)
#  include "./msvc/1200/typelist.h"
#  else
#  include "./reference/typelist.h"
#  endif
