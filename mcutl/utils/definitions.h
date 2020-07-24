#pragma once

#ifdef MCUTL_TEST
#	define MCUTL_NOEXCEPT
#else
#	define MCUTL_NOEXCEPT noexcept
#endif
