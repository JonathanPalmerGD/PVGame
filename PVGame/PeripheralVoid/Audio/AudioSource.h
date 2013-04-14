#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include "AL/al.h"
#include "AL/alc.h"

class AudioSource
{
private:
	float* position;//x, y, and z positions respectively

	//Audio data and information
	char* data;
	int channel;
	int sampleRate;
	int bps;
	int size;

	//OpenAL information
	unsigned int sourceid;
	unsigned int bufferid;
	unsigned int format;
	ALint state;

	//Private helper functions
	bool isBigEndian();
	int convertToInt(char* buffer,int len);

	bool initialized;
public:
	static enum FILE_TYPE{WAV = 1000, MP3 = 1001, FLAC = 1002};

	//void *operator new(std::size_t ObjectSize)
	//{
	//	return memPool->GetMemory(ObjectSize);
	//}

	AudioSource(void);
	~AudioSource(void);
	bool initialize(const char* fileName, FILE_TYPE type);
	char* loadWAV(const char* fn,int& chan,int& samplerate,int& bps,int& size);
	bool is3D();

	void setPosition(float x, float y, float z);
	void setPosition(float* newPosition);
	void move(float x, float y, float z);
	void move(float* amount);

	float* getPosition();
	float getX();
	float getY();
	float getZ();

	void play();
	void playAt(float x, float y, float z);
	void playAt(float* position);
	void playAtAndMoveTo(float x, float y, float z);
	void playAtAndMoveTo(float* position);
	void setLooping(bool);
	void stop();
	void pause();
	void resume();
	void restart();
	void restartAndPlay();

	bool isPlaying();
	unsigned int getSourceID();

	bool Initialized();
};

