
#include "Source.hpp"
#include "Logger.h"

void Source::setLooping(bool looping) {
    
    // Update flag
    if (looping == this->looping)
        return;
    this->looping = looping;
    
    // If the source is playing, need to notify OpenAL
    if (binding) {
        
        // Buffered source loop can be managed by OpenAL
        if (!sound.sound()->sampler) {
            alSourcei(binding->handle, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
            return;
        }
        
        // For streamed source, force update
        if (looping)
            sound.sound()->queue();
        // Note: disabling looping does not unqueue unprocessed buffers
    }

}

bool Source::isLooping() const {
    return looping;
}

void Source::setGain(float gain)
{
    _gain = gain;
    if(binding)
        alSourcef(binding->handle, AL_GAIN, gain);
}

void Source::setPitch(float pitch)
{
    _pitch = pitch;
    if(binding)
        alSourcef(binding->handle, AL_PITCH, pitch);
}

void Source::play() {

    // Make sure the sound is stopped
    stop();

    // Find available source
    for (Listener::Binding * b : listener->bindings)
        if (!b->source) {
            binding = b;
            break;
        }
    
    // If no source was found, ignore call
    if (!binding) {
        // TODO discard another sound with smaller priority
        LOG("warning: source limit reached");
        return;
    }
    
    // Attach sound
    if (!sound.sound()->attach(this)) {
        binding = nullptr;
        return;
    }
    
    // Enable loop for buffered sounds (streamed sound lopp will be handled in sound update)
    if (!sound.sound()->sampler)
        alSourcei(binding->handle, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    
    // Define properties
    alSource3f(binding->handle, AL_POSITION, _position.x(), _position.y(), _position.z());
    alSource3f(binding->handle, AL_VELOCITY, _velocity.x(), _velocity.y(), _velocity.z());
    alSourcef(binding->handle, AL_GAIN, _gain);
    alSourcef(binding->handle, AL_PITCH, _pitch);

    // alSource3f(handle, AL_DIRECTION, direction.x, direction.y, direction.z);
    // alSourcef(handle, AL_PITCH, 1); in 0.5 .. 2.0
    // TODO attentuation AL_CONE_INNER_ANGLE, AL_CONE_OUTER_ANGLE, AL_REFERENCE_DISTANCE, AL_ROLLOFF_FACTOR, AL_MAX_DISTANCE
    // TODO gain AL_GAIN, AL_MIN_GAIN, AL_MAX_GAIN, AL_CONE_OUTER_GAIN
    // TODO AL_SEC_OFFSET, AL_SAMPLE_OFFSET, AL_BYTE_OFFSET
    
    // Start playback
    binding->source = this;
    alSourcePlay(binding->handle);
}

void Source::pause() {
    if (isPlaying() && !paused) {
        alSourcePause(binding->handle);
        paused = true;
    }
}

void Source::resume() {
    if (paused) {
        alSourcePlay(binding->handle);
        paused = false;
    }
}


void Source::stop() {
    // Stop and unregister source
    if (binding) {
        alSourceStop(binding->handle);
        sound.sound()->detach();

        binding->source = nullptr;
        binding = nullptr;
        paused = false;
    }
    
    // If this sound was released, destroy it
    if (released) {
        listener->sources.erase(iterator);
        delete this;
    }
}

bool Source::isPlaying() const {
    return binding;
}

bool Source::isPaused() const {
    return paused;
}

void Source::release() {
    released = true;
    if (!isPlaying())
        stop();
}

Source::Source(Listener * listener) : listener(listener), released(false),
    binding(nullptr), paused(false),
    looping(false) {}
