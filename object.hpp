#pragma once
#include "unit_helper.hpp"

namespace content {	namespace unit {

// 위치와 고유 아이디 소유
class Object
{
public:
	Object() {}
	virtual ~Object() {}

protected:
	virtual void update() {}

private:
	uint64_t _uniqueId{ 0 };
	vector2 _pos{};
	Species _species{ Species::SPECIES_NONE };
};

}}
