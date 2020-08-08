#pragma once

namespace mcutl::types
{

struct noncopyable
{
	noncopyable() = default;
	noncopyable(const noncopyable&) = delete;
	noncopyable& operator=(const noncopyable&) = delete;
	noncopyable(noncopyable&&) = default;
	noncopyable& operator=(noncopyable&&) = default;
};

struct noncopymovable : noncopyable
{
	noncopymovable() = default;
	noncopymovable(noncopymovable&&) = delete;
	noncopymovable& operator=(noncopymovable&&) = delete;
};

struct static_class : noncopymovable
{
	static_class() = delete;
};

} //namespace mcutl::types
