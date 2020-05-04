#ifndef SPEECH_ENGINE_H
#define SPEECH_ENGINE_H

#include <string>
#include <mutex>

typedef struct SpeechChannelRecord  SpeechChannelRecord;
typedef SpeechChannelRecord *SpeechChannel;

class SpeechEngine
{
public:
    SpeechEngine(const std::string &voice1, const std::string &voice2);
    ~SpeechEngine();

    void say(const std::string &text);
    void say(const std::string &text, const std::string &translation);

private:
    void say(const std::string &text, SpeechChannel chan) const;
    void create_voice_channel(const std::string &voice, SpeechChannel &chan) const;
    static void speech_done_callback(SpeechChannel, void *ptr);

private:
    SpeechChannel channel;
    SpeechChannel channel_tr;

    std::mutex lock;
    std::string _translation;
};

#endif // SPEECH_ENGINE_H
