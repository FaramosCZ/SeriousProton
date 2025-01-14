#include "audio/sound.h"
#include "resources.h"

#include <cstring>

namespace sp {
namespace audio {

/*
    P<ResourceStream> stream = getResourceStream(name);
    if (!stream) stream = getResourceStream(name + ".wav");
*/
void SoundPlayback::play(const Sound& _sound, bool _loop)
{
    stop();
    loop = _loop;
    sound = &_sound;
    index = 0.0f;
    start();
}

void SoundPlayback::setVolume(float _volume)
{
    volume = _volume;
}

void SoundPlayback::setPitch(float _pitch)
{
    pitch = _pitch;
}

void SoundPlayback::onMixSamples(int16_t* stream, int sample_count)
{
    float index_offset = pitch * float(sound->samplerate) / 44100.0f;
    if (sound->channels == 1)
    {
        for(int idx=0; idx<sample_count; idx+=2)
        {
            if (size_t(index) >= sound->samples.size())
            {
                if (loop)
                {
                    index = 0.0f;
                }
                else
                {
                    stop();
                    return;
                }
            }

            int sample = float(sound->samples[int(index)]) * volume;
            stream[idx+0] = std::clamp(int(stream[idx+0] + sample), int(std::numeric_limits<int16_t>::min()), int(std::numeric_limits<int16_t>::max()));
            stream[idx+1] = std::clamp(int(stream[idx+1] + sample), int(std::numeric_limits<int16_t>::min()), int(std::numeric_limits<int16_t>::max()));

            index += index_offset;
        }
    }
    else if (sound->channels == 2)
    {
        for(int idx=0; idx<sample_count; idx+=2)
        {
            if (size_t(index) * 2 >= sound->samples.size()) {
                if (loop)
                {
                    index = 0.0f;
                }
                else
                {
                    stop();
                    return;
                }
            }

            int sample_left = float(sound->samples[int(index) * 2]) * volume;
            int sample_right = float(sound->samples[int(index) * 2 + 1]) * volume;
            stream[idx+0] = std::clamp(int(stream[idx+0] + sample_left), int(std::numeric_limits<int16_t>::min()), int(std::numeric_limits<int16_t>::max()));
            stream[idx+1] = std::clamp(int(stream[idx+1] + sample_right), int(std::numeric_limits<int16_t>::min()), int(std::numeric_limits<int16_t>::max()));

            index += index_offset;
        }
    }
}

Sound::Sound(const string& resource_name)
{
    auto stream = getResourceStream(resource_name);
    if (!stream)
        return;
    char chunk_id[4];
    uint32_t chunk_size;

    if (stream->read(chunk_id, sizeof(chunk_id)) != sizeof(chunk_id)) return;
    if (strncmp(chunk_id, "RIFF", 4) != 0) return;
    if (stream->read(&chunk_size, sizeof(chunk_size)) != sizeof(chunk_size)) return;
    if (stream->read(chunk_id, sizeof(chunk_id)) != sizeof(chunk_id)) return;
    if (strncmp(chunk_id, "WAVE", 4) != 0) return;
    while(true)
    {
        if (stream->read(chunk_id, sizeof(chunk_id)) != sizeof(chunk_id)) return;
        if (stream->read(&chunk_size, sizeof(chunk_size)) != sizeof(chunk_size)) return;

        if (strncmp(chunk_id, "fmt ", 4) == 0)
        {
            if (chunk_size < 16) return;
            
            uint16_t fmt_format;
            uint16_t fmt_channels;
            uint32_t fmt_samplerate;

            if (stream->read(&fmt_format, sizeof(fmt_format)) != sizeof(fmt_format)) return;
            if (stream->read(&fmt_channels, sizeof(fmt_channels)) != sizeof(fmt_channels)) return;
            if (stream->read(&fmt_samplerate, sizeof(fmt_samplerate)) != sizeof(fmt_samplerate)) return;
            char tmp[6]; //skip the ByteRate and BlockAlign
            if (stream->read(tmp, sizeof(tmp)) != sizeof(tmp)) return;
            uint16_t fmt_bps;
            if (stream->read(&fmt_bps, sizeof(fmt_bps)) != sizeof(fmt_bps)) return;

            chunk_size -= 16;
            while(chunk_size > 0) {
                if (stream->read(tmp, 1) != 1) return;
                chunk_size--;
            }

            //Check if we have uncompressed PCM
            if (fmt_format != 1) return;
            //Check if we have 16 bit samples
            if (fmt_bps != 16) return;
            if (fmt_channels < 1 || fmt_channels > 2) return;

            samplerate = fmt_samplerate;
            channels = fmt_channels;
        }
        else if (strncmp(chunk_id, "data", 4) == 0)
        {
            samples.resize(chunk_size / 2);
            if (stream->read(samples.data(), chunk_size) != chunk_size) return;
        }
        else
        {
            char tmp;
            while(chunk_size > 0) {
                if (stream->read(&tmp, 1) != 1) return;
                chunk_size--;
            }
        }
    }
}

float Sound::getDuration()
{
    return float(samples.size()) / float(samplerate) / float(channels);
}

int Sound::getChannelCount()
{
    return channels;
}

}//namespace audio
}//namespace sp
