#include "speech_engine.h"
#include <ApplicationServices/ApplicationServices.h>


SpeechEngine::SpeechEngine(const std::string &voice) :
    channel(create_channel(voice))
{
//    if (channel != nullptr) {
//        auto callback_ptr = &speech_done_callback;
//        auto cbf = CFNumberCreate(nullptr, kCFNumberLongType, &callback_ptr);
//        SetSpeechProperty(channel, kSpeechSpeechDoneCallBack, cbf);
//        CFRelease(cbf);
//        SpeechEngine *this_ptr = this;
//        cbf = CFNumberCreate(nullptr, kCFNumberLongType, &this_ptr);
//        SetSpeechProperty(channel, kSpeechRefConProperty, cbf);
//        CFRelease(cbf);
//    }
}

SpeechEngine::~SpeechEngine()
{
    DisposeSpeechChannel(channel);
}

void SpeechEngine::say(const std::string &text)
{
    say(text, channel);
}

void SpeechEngine::say(const std::string &text, SpeechChannel chan)
{
    auto cs = CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(text.data()), text.size() * sizeof(char), kCFStringEncodingUTF8, false);
    SpeakCFString(chan, cs, nullptr);
    CFRelease(cs);
}

SpeechChannel SpeechEngine::create_channel(const std::string &voice)
{
    SInt16 numVoices;
    if (CountVoices(&numVoices) != noErr) {
        return nullptr;
    }
    SpeechChannel chan;
    VoiceSpec spec;
    VoiceDescription descr;
    for (int i = 0; i < numVoices; ++i) {
        if (GetIndVoice(i, &spec) != noErr) {
            continue;
        }
        if (GetVoiceDescription(&spec, &descr, sizeof(VoiceDescription)) != noErr) {
            continue;
        }
        if (voice.compare(0, voice.size(), reinterpret_cast<const char *>(descr.name + 1), *descr.name) == 0) {
            return NewSpeechChannel(&spec, &chan) == noErr ? chan : nullptr;
        }
    }
    return NewSpeechChannel(nullptr, &chan) == noErr ? chan : nullptr;
}

//void SpeechEngine::speech_done_callback(SpeechChannel, void *ptr)
//{
//    auto self = reinterpret_cast<SpeechEngine *>(ptr);
//}
