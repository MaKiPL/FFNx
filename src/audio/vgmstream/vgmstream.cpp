/****************************************************************************/
//    Copyright (C) 2009 Aali132                                            //
//    Copyright (C) 2018 quantumpencil                                      //
//    Copyright (C) 2018 Maxime Bacoux                                      //
//    Copyright (C) 2020 Julian Xhokaxhiu                                   //
//    Copyright (C) 2020 myst6re                                            //
//    Copyright (C) 2020 Chris Rizzitello                                   //
//    Copyright (C) 2020 John Pritchard                                     //
//                                                                          //
//    This file is part of FFNx                                             //
//                                                                          //
//    FFNx is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU General Public License as published by  //
//    the Free Software Foundation, either version 3 of the License         //
//                                                                          //
//    FFNx is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of        //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
//    GNU General Public License for more details.                          //
/****************************************************************************/

#include "vgmstream.h"

namespace SoLoud
{
	VGMStreamInstance::VGMStreamInstance(VGMStream* aParent)
	{
		mParent = aParent;
		mOffset = 0;
	}

	unsigned int VGMStreamInstance::getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize)
	{
		unsigned int offset = 0;
		unsigned int i, j, k;

		for (i = 0; i < aSamplesToRead; i += 512)
		{
			sample_t* tmp = new sample_t[512 * mChannels];
			unsigned int blockSize = (aSamplesToRead - i) > 512 ? 512 : aSamplesToRead - i;
			offset += (unsigned int)render_vgmstream(tmp, blockSize, mParent->stream);

			for (j = 0; j < blockSize; j++)
			{
				for (k = 0; k < mChannels; k++)
				{
					aBuffer[k * aSamplesToRead + i + j] = tmp[j * mChannels + k] / (float)INT16_MAX;
				}
			}

			delete[] tmp;
		}
		mOffset += offset;
		return offset;
	}

	result VGMStreamInstance::rewind()
	{
		seek_vgmstream(mParent->stream, 0);

		mOffset = 0;
		mStreamPosition = 0.0f;
		return 0;
	}

	bool VGMStreamInstance::hasEnded()
	{
		if (!(mFlags & AudioSourceInstance::LOOPING) && mOffset >= mParent->mSampleCount)
		{
			return 1;
		}
		return 0;
	}

	VGMStream::VGMStream()
	{
		mSampleCount = 0;
	}

	VGMStream::~VGMStream()
	{
		stop();

		close_vgmstream(stream);
	}

	result VGMStream::load(const char* aFilename)
	{
		if (aFilename == 0)
			return INVALID_PARAMETER;

		stop();

		stream = init_vgmstream(aFilename);

		mBaseSamplerate = (float)stream->sample_rate;
		mSampleCount = (unsigned int)stream->num_samples;
		mChannels = stream->channels;
		if (stream->loop_flag) setLooping(true);

		return SO_NO_ERROR;
	}

	AudioSourceInstance* VGMStream::createInstance()
	{
		return new VGMStreamInstance(this);
	}

	double VGMStream::getLength()
	{
		if (mBaseSamplerate == 0)
			return 0;

		return mSampleCount / mBaseSamplerate;
	}
};
