#include "AudioListener.h"

/* Constructor
 *
 * Sets variables to default values
 */
AudioListener::AudioListener(void)
{
	position = new float[3];
	orientation = new float[6];
	alGetListenerf(AL_GAIN, &gain);
}

/* Destructor
 *
 * Frees all used memory
 */
AudioListener::~AudioListener(void)
{
	delete[] position;
	delete[] orientation;
}

/* setPosition()
 *
 * manually sets the x y and z of the position
 */
void AudioListener::setPosition(float x, float y, float z)
{
	position[0] = x;
	position[1] = y;
	position[2] = z;
	alListenerfv(AL_POSITION, position);
}

/* setPosition()
 *
 * sets the position array to a new array
 */
void AudioListener::setPosition(float* newPosition)
{
	position = newPosition;
	alListenerfv(AL_POSITION, position);
}

/* getPosition()
 *
 * returns pointer to the start of the position array
 */
float* AudioListener::getPosition()
{
	return position;
}

/* getX()
 *
 * returns the x position
 */
float AudioListener::getX()
{
	return position[0];
}

/* getY()
 *
 * returns the y position
 */
float AudioListener::getY()
{
	return position[1];
}

/* getZ()
 *
 * returns the z position
 */
float AudioListener::getZ()
{
	return position[2];
}

/* move()
 *
 * moves the AudioListener in space
 */
void AudioListener::move(float x, float y, float z)
{
	position[0] += x;
	position[1] += y;
	position[2] += z;
	alListenerfv(AL_POSITION, position);
}

/* move()
 *
 * moves the AudioListener in space
 */
void AudioListener::move(float* amount)
{
	position[0] += amount[0];
	position[1] += amount[1];
	position[2] += amount[2];
	alListenerfv(AL_POSITION, position);
}

/* setForward()
 *
 * sets the forward vector in the orientation
 */
void AudioListener::setForward(float x, float y, float z)
{
	orientation[0] = x;
	orientation[1] = y;
	orientation[2] = z;
	alListenerfv(AL_ORIENTATION,orientation);
}

/* setForward()
 *
 * sets the forward vector in the orientation
 */
void AudioListener::setForward(float* fwd)
{
	orientation[0] = fwd[0];
	orientation[1] = fwd[1];
	orientation[2] = fwd[2];
	alListenerfv(AL_ORIENTATION,orientation);
}

/* setUp()
 *
 * sets the up vector in the orientation
 */
void AudioListener::setUp(float x, float y, float z)
{
	orientation[3] = x;
	orientation[4] = y;
	orientation[5] = z;
	alListenerfv(AL_ORIENTATION,orientation);
}

/* setUp()
 *
 * sets the up vector in the orientation
 */
void AudioListener::setUp(float* up)
{
	orientation[3] = up[0];
	orientation[4] = up[1];
	orientation[5] = up[2];
	alListenerfv(AL_ORIENTATION,orientation);
}

/* setOrientation()
 *
 * sets the forward and up vector in the orientation
 */
void AudioListener::setOrientation(float fX, float fY, float fZ, float uX, float uY, float uZ)
{
	orientation[0] = fX;
	orientation[1] = fY;
	orientation[2] = fZ;
	orientation[3] = uX;
	orientation[4] = uY;
	orientation[5] = uZ;
	alListenerfv(AL_ORIENTATION,orientation);
}

/* setOrientation()
 *
 * sets the orientation to a new array
 */
void AudioListener::setOrientation(float* newOrientation)
{
	orientation = newOrientation;
	alListenerfv(AL_ORIENTATION,orientation);
}

/* getOrientation()
 *
 * 0-2 are the forward vector, 3-5 are the up vector
 *
 * returns the orientation
 */
float* AudioListener::getOrientation()
{
	return orientation;
}

/* mute()
 *
 * sets the gain of the listener to 0, effectively making it deaf
 */
void AudioListener::mute()
{
	alListenerf(AL_GAIN, 0.0f);
}

/* unmute()
 *
 * resets the gain to the origional value
 */
void AudioListener::unmute()
{
	alListenerf(AL_GAIN, gain);
}

/* isMuted()
 *
 * returns if the listener is a deafy or a normal
 */
bool AudioListener::isMuted()
{
	float tG;
	alGetListenerf(AL_GAIN,&tG);
	return tG==0.0f;
}

void AudioListener::setGain(float ngain)
{
	this->gain = ngain;
	alListenerf(AL_GAIN, gain);
}