#include "AudioSource.h"

/* Constructor
 *
 * Sets the position to (0,0,0);
 * Sets all other data to default values
 */
AudioSource::AudioSource(void)
{
	position    = new float[3];
	position[0] = 0;
	position[1] = 0;
	position[2] = 0;
	data        = NULL;
	channel     = 0;
	sampleRate  = 0;
	bps         = 0;
	size        = 0;
	sourceid    = 0;
	bufferid    = 0;
	format      = 0;
	state       = AL_INITIAL;
}

/* initialize()
 *
 * Loads in an audio file and initializes the source inside of OpenAL.
 * This MUST be called before the AudioSource can be used
 *
 * Returns whether the AudioSource was successfully initialized or not
 */
bool AudioSource::initialize(const char* fileName, FILE_TYPE type)
{
	//Load in data
	switch(type)
	{
	case WAV:
		data = loadWAV(fileName, channel, sampleRate, bps, size);
		break;
	case MP3://May be able to add MP3 Support later 
		data = NULL;
		break;
	case FLAC://May be able to add FLAC Support later
		data = NULL;
		break;
	default:
		data = NULL;
		break;
	}

	//Set variables and set up OpenAL with the source
	if(data != NULL)
	{
		alGenBuffers(1,&bufferid);

		//NOTE: The sound will only be 3D if the format is MONO
		if(channel==1)
		{
			if(bps==8) 
				format=AL_FORMAT_MONO8;
			else
				format=AL_FORMAT_MONO16;
		}
		else
		{
			if(bps==8)
				format=AL_FORMAT_STEREO8;
			else
				format=AL_FORMAT_STEREO16;
		}

		//Generate the audio information in OpenAL
		alBufferData(bufferid,format,data,size,sampleRate);
		alGenSources(1,&sourceid);

		//Set Information about the Source in OpenAL
		alSourcei(sourceid,AL_BUFFER,bufferid);
		alSource3f(sourceid,AL_POSITION,position[0],position[1],position[2]);
		alSourcei(sourceid,AL_LOOPING,AL_FALSE);
		alSourcei(sourceid,AL_SOURCE_RELATIVE, AL_FALSE);
		alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
		return true;
	}
	return false;
}

/* Destructor
 *
 * Cleans up all used memory in OpenAL and regular memory
 */
AudioSource::~AudioSource(void)
{
	alDeleteSources(1,&sourceid);
	alDeleteBuffers(1,&bufferid);
	delete[] data;
	delete[] position;
}

/* isBigEndian()
 *
 * Checks to see if the system the program is exectuted 
 * in is Big Endian or Little Endian
 *
 * returns true if the system is a Big Endian System
 */
bool AudioSource::isBigEndian()
{
	int a=1;
	return !((char*)&a)[0];
}

/* convertToInt
 *
 * Converts a byte array into an integer
 *
 * returns the integer
 */
int AudioSource::convertToInt(char* buffer,int len)
{
	int a=0;
	if(!isBigEndian())
		for(int i=0;i<len;i++)
			((char*)&a)[i]=buffer[i];
	else
		for(int i=0;i<len;i++)
			((char*)&a)[3-i]=buffer[i];	
	return a;
}

/*loadWAV
 *
 * Loads the sound data from a WAV file into a char* and 
 * fills all the necessary variables with the information
 * from the file
 *
 * returns a char array containing the raw audio data from the WAV file
 */
char* AudioSource::loadWAV(const char* fn,int& chan,int& samplerate,int& bps,int& size)
{
	char buffer[4];
	std::ifstream in(fn,std::ios::binary);
	in.read(buffer,4);
	
	if(strncmp(buffer,"RIFF",4)!=0)
		return NULL;
	
	in.read(buffer,4);
	in.read(buffer,4);	//WAVE
	
	if(strncmp(buffer,"WAVE",4)!=0)
		return NULL;
	
	in.read(buffer,4);	//fmt 
	
	if(strncmp(buffer,"fmt ",4)!=0)
		return NULL;
	
	in.read(buffer,4);	//16
	in.read(buffer,2);	//1
	in.read(buffer,2);
	chan=convertToInt(buffer,2);
	in.read(buffer,4);
	samplerate=convertToInt(buffer,4);
	in.read(buffer,4);
	in.read(buffer,2);
	in.read(buffer,2);
	bps=convertToInt(buffer,2);
	in.read(buffer,4);	//data
	in.read(buffer,4);
	
	size=convertToInt(buffer,4);
	char* data=new char[size];
	in.read(data,size);
	return data;	
}

bool AudioSource::is3D()
{
	return format == AL_FORMAT_MONO8 || format == AL_FORMAT_MONO16;
}

/* setPosition()
 *
 * Sets the position of the audio source
 */
void AudioSource::setPosition(float x, float y, float z)
{
	position[0] = x;
	position[1] = y;
	position[2] = z;
	alSource3f(sourceid,AL_POSITION,position[0],position[1],position[2]);
}

/* setPosition()
 *
 * Sets the position of the audio source
 */
void AudioSource::setPosition(float* newPosition)
{
	position = newPosition;
	alSource3f(sourceid,AL_POSITION,position[0],position[1],position[2]);
}

/* move()
 *
 * moves the position of the audioSource by an amount
 */
void AudioSource::move(float x, float y, float z)
{
	position[0] += x;
	position[1] += y;
	position[2] += z;
	alSource3f(sourceid,AL_POSITION,position[0],position[1],position[2]);
}

/* move()
 *
 * moves the position of the audio source by an amount
 */
void AudioSource::move(float* amount)
{
	position[0] += amount[0];
	position[1] += amount[1];
	position[2] += amount[2];
	alSource3f(sourceid,AL_POSITION,position[0],position[1],position[2]);
}

/* getPosition()
 *
 * returns the position of the audioSource
 */
float* AudioSource::getPosition()
{
	return position;
}

/* getX()
 *
 * returns the x position of the audio source
 */
float AudioSource::getX()
{
	return position[0];
}

/* getY()
 *
 * returns the Y position of the audio source
 */
float AudioSource::getY()
{
	return position[1];
}

/* getZ()
 *
 * returns the Z position of the audio source
 */
float AudioSource::getZ()
{
	return position[2];
}

/* play()
 *
 * plays the audio at the current position
 */	
void AudioSource::play()
{
	alSourcePlay(sourceid);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}

/* playAt()
 *
 * plays the audio at a position
 * This position does not have to be the current position of the audio source
 */
void AudioSource::playAt(float x, float y, float z)
{
	alSource3f(sourceid,AL_POSITION, x, y, z);
	alSourcePlay(sourceid);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}

/* playAt()
 *
 * plays the audio at a position
 * This position does not have to be the current position of the audio source
 */
void AudioSource::playAt(float* position)
{
	alSource3f(sourceid,AL_POSITION,position[0],position[1],position[2]);
	alSourcePlay(sourceid);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}

/* playAtAndMoveTo
 *
 * Sets the position of the audio source and
 * then plays the audio at that location
 */
void AudioSource::playAtAndMoveTo(float x, float y, float z)
{
	setPosition(x, y, z);
	alSourcePlay(sourceid);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}

/* playAtAndMoveTo
 *
 * Sets the position of the audio source and
 * then plays the audio at that location
 */
void AudioSource::playAtAndMoveTo(float* position)
{
	setPosition(position);
	alSourcePlay(sourceid);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}

/* setLooping
 *
 * sets whether the audio should loop or not
 */
void AudioSource::setLooping(bool looping)
{
	if(looping)
		alSourcei(sourceid,AL_LOOPING,AL_TRUE);
	else
		alSourcei(sourceid,AL_LOOPING,AL_FALSE);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}
 
/* stop()
 *
 * stops the audio if its playing
 */
void AudioSource::stop()
{
	alSourceStop(sourceid);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}

/* pause()
 *
 * pauses the audio if playing
 */
void AudioSource::pause()
{
	alSourcePause(sourceid);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}

/* resume()
 *
 * resumes playing the audio
 */
void AudioSource::resume()
{
	alSourcePlay(sourceid);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}

/* restart()
 *
 * rewinds the audio to the beginning and stops play
 */
void AudioSource::restart()
{
	alSourceRewind(sourceid);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}

/* restartAndPlay()
 *
 * rewinds the audio to the beginning and plays it
 */
void AudioSource::restartAndPlay()
{
	alSourceRewind(sourceid);
	alSourcePlay(sourceid);
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
}

/* isPlaying()
 *
 * updates the state of the audio source and checks to see if it is playing
 *
 * returns if the audio is playing
 */
bool AudioSource::isPlaying()
{
	alGetSourcei(sourceid, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

/* getSourceID()
 *
 * returns the id of the audio source in OpenAL
 */
unsigned int AudioSource::getSourceID()
{
	return sourceid;
}