#pragma once
#include "Sampler2D.h"
class Sampler2DDX :
    public Sampler2D
{
public:
	Sampler2DDX();
	virtual ~Sampler2DDX();
	virtual void setMagFilter(FILTER filter);
	virtual void setMinFilter(FILTER filter);
	virtual void setWrap(WRAPPING s, WRAPPING t);
};

