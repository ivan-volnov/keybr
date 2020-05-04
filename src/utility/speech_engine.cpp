#include "speech_engine.h"
#include <ApplicationServices/ApplicationServices.h>


SpeechEngine::SpeechEngine(const std::string &voice1, const std::string &voice2)
{
    create_voice_channel(voice1, channel);
    create_voice_channel(voice2, channel_tr);

    auto callback_ptr = &speech_done_callback;
    auto cbf = CFNumberCreate(nullptr, kCFNumberLongType, &callback_ptr);
    SetSpeechProperty(channel, kSpeechSpeechDoneCallBack, cbf);
    CFRelease(cbf);
    SpeechEngine *this_ptr = this;
    cbf = CFNumberCreate(nullptr, kCFNumberLongType, &this_ptr);
    SetSpeechProperty(channel, kSpeechRefConProperty, cbf);
    CFRelease(cbf);
}

SpeechEngine::~SpeechEngine()
{
    DisposeSpeechChannel(channel);
    DisposeSpeechChannel(channel_tr);
}

void SpeechEngine::say(const std::string &text)
{
    say(text, channel);
}

void SpeechEngine::say(const std::string &text, const std::string &translation)
{
    std::lock_guard guard(lock);
    _translation = translation;
    say(text);
}

void SpeechEngine::say(const std::string &text, SpeechChannel chan) const
{
    auto cs = CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(text.data()), text.size() * sizeof(char), kCFStringEncodingUTF8, false);
    SpeakCFString(chan, cs, nullptr);
    CFRelease(cs);
}

void SpeechEngine::create_voice_channel(const std::string &voice, SpeechChannel &chan) const
{
    SInt16 numVoices;
    if (CountVoices(&numVoices) != noErr) {
        return;
    }
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
            NewSpeechChannel(&spec, &chan);
            return;
        }
    }
    NewSpeechChannel(nullptr, &chan);
}

void SpeechEngine::speech_done_callback(SpeechChannel, void *ptr)
{
    auto self = reinterpret_cast<SpeechEngine *>(ptr);
    std::lock_guard guard(self->lock);
    if (!self->_translation.empty()) {
        self->say(self->_translation, self->channel_tr);
        self->_translation.clear();
    }
}
