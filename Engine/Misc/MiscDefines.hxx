#pragma once

#define RAPHAEL_NONCOPYABLE(Name) \
	Name(Name&&) = delete; \
	Name(const Name&) = delete; \
	Name& operator=(const Name&) = delete; \
	Name& operator=(Name&&) = delete;

