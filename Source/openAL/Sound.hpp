
#ifndef GLOW_SOUND_HPP
#define GLOW_SOUND_HPP

#include "Sampler.hpp"
#include <vector>

class Source;

class Sound {
    friend class Listener;
    friend class Source;

public:
    
    Sound(Sound const &) = delete;
    Sound & operator=(Sound const &) = delete;

    Sound(Sampler * sampler, bool stream);
    ~Sound();
    
    bool isStream() const;
    // TODO keep infos about sound (format, duration...)?
    
    // TODO release?
    
private:
    Sampler * sampler;
    
    uint32_t fill(ALuint buffer, uint32_t samples, bool loop);
    
    void unqueue();
    void queue();
    
    bool attach(Source * source);
    void detach();
    
    void update();
    
    ALuint handle;
    
    Source* source;
    static const int NUM_ROUND = 8;
    ALuint cycle[NUM_ROUND];
    std::vector<ALuint> available;
};

#endif
