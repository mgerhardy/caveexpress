#include "engine/common/Config.h"

#ifndef APPNAME
#error "No APPNAME set"
#endif

#ifndef APPNAME_FULL
#error "No APPNAME_FULL set"
#endif

#ifndef VERSION
#error "No VERSION set"
#endif

#ifndef VERSION_CODE
#error "No VERSION_CODE set"
#endif

#define APPFULLNAME APPNAME " " VERSION
#define FULLVERSION VERSION "-" VERSION_CODE
