#ifndef MATHLIB_DBG_H
#define MATHLIB_DBG_H

// You can redefine MathlibAssert to some other macro.
// Otherwise, if MATHLIB_USE_C_ASSERT defined, it will use <cassert> assert
// Otherwise, tier0's Assert is used (linking with tier0 is required).
#ifndef MathlibAssert

#ifdef MATHLIB_USE_C_ASSERT

// C assert
#include <cassert>
#define MathlibAssert assert

#else

// Tier0 Assert
#include <tier0/dbg.h>
#define MathlibAssert Assert

#endif // ifdef MATHLIB_USE_C_ASSERT

#endif // ifndef MathlibAssert

#endif // ifndef MATHLIB_DBG_H
