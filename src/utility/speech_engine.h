#ifndef SPEECH_ENGINE_H
#define SPEECH_ENGINE_H

#include <string>

typedef struct SpeechChannelRecord  SpeechChannelRecord;
typedef SpeechChannelRecord *SpeechChannel;

class SpeechEngine
{
public:
    SpeechEngine();
    ~SpeechEngine();

    void say(const std::string &text);

private:
    SpeechChannel channel;
};

#endif // SPEECH_ENGINE_H
