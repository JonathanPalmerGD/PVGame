#pragma once
#include "AL/al.h"
#include "AL/alc.h"

class AudioListener
{
private:
	float* position; //3 element array: x, y, z
	float* orientation;//6 element array: 0-2 are forward vectors x, y and z and 3-5 are the up vectors x, y, and z
	ALfloat gain;

public:
	AudioListener(void);
	~AudioListener(void);

	void setPosition(float x, float y, float z);
	void setPosition(float* newPosition);
	float* getPosition();
	float getX();
	float getY();
	float getZ();

	void move(float x, float y, float z);
	void move(float* amount);

	void setForward(float x, float y, float z);
	void setForward(float* fwd);

	void setUp(float x, float y, float z);
	void setUp(float* up);

	void setOrientation(float fX, float fY, float fZ, float uX, float uY, float uZ);
	void setOrientation(float* newOrientation);
	float* getOrientation();

	void mute();
	void unmute();
	bool isMuted();

	void setGain(float gain);
};

