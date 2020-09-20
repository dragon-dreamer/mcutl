#pragma once

namespace mcutl::types
{

enum class endian
{
	little = __ORDER_LITTLE_ENDIAN__,
	big    = __ORDER_BIG_ENDIAN__,
	native = __BYTE_ORDER__
};

} //namespace mcutl::types
