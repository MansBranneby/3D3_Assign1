#pragma once
#include "Texture2D.h"
class Texture2DDX :
    public Texture2D
{
public:
	Texture2DDX();
	~Texture2DDX();
	virtual int loadFromFile(std::string filename);
	virtual void bind(unsigned int slot);
};

