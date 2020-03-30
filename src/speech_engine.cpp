#include "speech_engine.h"
#include <ApplicationServices/ApplicationServices.h>


SpeechEngine::SpeechEngine()
{
    NewSpeechChannel(nullptr, &channel);
}

SpeechEngine::~SpeechEngine()
{
    DisposeSpeechChannel(channel);
}

void SpeechEngine::say(const std::string &text)
{
    auto cs = CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const UInt8 *>(text.data()), text.size() * sizeof(char), kCFStringEncodingUTF8, false);
    SpeakCFString(channel, cs, nullptr);
    CFRelease(cs);
}
