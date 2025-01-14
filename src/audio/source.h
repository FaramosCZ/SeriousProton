#ifndef SP2_AUDIO_SOURCE_H
#define SP2_AUDIO_SOURCE_H

#include <stdint.h>
#include "nonCopyable.h"


class Engine;
namespace sp {
namespace audio {

/** Base class for objects that want to output sound.
    Generally, this is used by the sound and music modules to output their audio.
    But if you want to create custom generated audio output, this class can be used.
 */
class Source : sp::NonCopyable
{
public:
    virtual ~Source();

    void start();
    bool isPlaying();
    void stop();

protected:
    virtual void onMixSamples(int16_t* stream, int sample_count) = 0;

private:
    bool active = false;
    Source* next;
    Source* previous;
    
private:
    static void startAudioSystem();
    static void onAudioCallback(int16_t* stream, int sample_count);
    
    friend class ::Engine;
    friend class MySFMLStream;
};

}//namespace audio
}//namespace sp

#endif//SP2_AUDIO_AUDIOSOURCE_H