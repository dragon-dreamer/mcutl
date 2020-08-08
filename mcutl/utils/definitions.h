#pragma once

#ifdef MCUTL_TEST
#	define MCUTL_NOEXCEPT noexcept(false)
#else
#	define MCUTL_NOEXCEPT noexcept
#endif

#define MCUTL_STATIC_FORCEINLINE __attribute__((always_inline)) static __inline
#define MCUTL_STATIC_NAKED_NOINLINE __attribute__((noinline, naked)) static
