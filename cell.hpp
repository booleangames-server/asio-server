#pragma once
#include <vector>
#include "unit_helper.hpp"

namespace content {	namespace unit {

class Object;
class Cell
{
public:
	Cell() {}
	virtual ~Cell() {}

public:
	virtual void update() = 0;

private:
	vector2 _pos;
	std::vector<Object*> _objects;
};

}}