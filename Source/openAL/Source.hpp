
#ifndef GLOW_SOURCE_HPP
#define GLOW_SOURCE_HPP

#include "Listener.hpp"
#include "Sound.hpp"

class Source
{
    friend class Listener;
    friend class Sound;
public:

    Source(Source const &) = delete;
    Source & operator=(Source const &) = delete;

    void setLooping(bool looping);
    bool isLooping() const;

    void setGain(float);
    void setPitch(float);
    
    // TODO direction, pitch, attenuation, gain

    void play();
    void pause();
    void resume();
    void stop();

    bool isPlaying() const;
    bool isPaused() const;

    // Note: the source is released as soon as the sound ends
    void release();

    /** added from timengine */
    const core::vec3& position() const { return _position; }
    const core::vec3& velocity() const { return _velocity; }

    void setPosition(const core::vec3& p) { _position = p; }
    void setVelocity(const core::vec3& v) { _velocity = v; }

    const resource::SoundAsset& getSound() const { return sound; }

private:

    Source(Listener * listener);
    ~Source() = default;

    Listener * listener;
    std::list<Source *>::iterator iterator;
    bool released;

    Listener::Binding * binding;
    bool paused;

    resource::SoundAsset sound;
    bool looping;

    /** added from timengine */
    core::vec3 _velocity;
    core::vec3 _position;
    float _gain = 1, _pitch = 1;
};

#endif
