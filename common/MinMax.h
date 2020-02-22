#ifndef COMMON_MINMAX_H
#define COMMON_MINMAX_H

#undef min
#undef max
#undef clamp

#include <algorithm>

#if defined( min ) || defined( max )
#error "Don't define min and/or max, use std::min and std::max instead!"
#endif

using std::min;
using std::max;

//C++17 will have std::clamp, so make sure it matches the function declaration. - Solokiller
//Note: std::clamp will return const ref, which can produce dangling references. This version returns a copy.
template<typename T> T clamp( const T& val, const T& min, const T& max )
{
	return ( ( val > max ) ? max : ( ( val < min ) ? min : val ) );
}

#endif //COMMON_MINMAX_H
