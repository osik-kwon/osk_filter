#pragma once
#include "pole/pole.h"

namespace filter
{
	struct cfb_traits
	{
		typedef POLE::Storage storage_t;
		typedef POLE::Stream stream_t;
	};
}