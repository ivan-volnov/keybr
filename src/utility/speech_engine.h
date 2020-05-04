#ifndef SPEECH_ENGINE_H
#define SPEECH_ENGINE_H

#include <string>

typedef struct SpeechChannelRecord  SpeechChannelRecord;
typedef SpeechChannelRecord *SpeechChannel;

class SpeechEngine
{
public:
    SpeechEngine(const std::string &voice);
    ~SpeechEngine();

    void say(const std::string &text);

private:
    static void say(const std::string &text, SpeechChannel chan);
    static SpeechChannel create_channel(const std::string &voice);
//    static void speech_done_callback(SpeechChannel, void *ptr);

private:
    SpeechChannel channel;
};

#endif // SPEECH_ENGINE_H
