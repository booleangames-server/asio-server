#pragma once
namespace content {	namespace unit {

class Behavior
{
public:
	Behavior() {}
	virtual ~Behavior() {}

public:
	virtual void action() = 0;

private:

};

}}