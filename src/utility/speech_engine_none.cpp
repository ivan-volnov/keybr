#include "speech_engine.h"


SpeechEngine::SpeechEngine(const std::string &voice) :
    channel(create_channel(voice))
{

}

SpeechEngine::~SpeechEngine()
{

}

void SpeechEngine::say(const std::string &)
{

}

void SpeechEngine::say(const std::string &, SpeechChannel)
{

}

SpeechChannel SpeechEngine::create_channel(const std::string &)
{
    return nullptr;
}
