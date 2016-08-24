
#include "Listener.hpp"
#include "Source.hpp"
#include "Sound.hpp"

#include "core/Logger.h"

using namespace core;

Listener::Listener()
: device(nullptr), context(nullptr), efx(false)
{}

Listener::~Listener() {
    if (device) {
        if (context) {
            
            // Release resources
            for (Binding * binding : bindings) {
                alSourceStop(binding->handle);
                alDeleteSources(1, &binding->handle);
                delete binding;
            }
            for (Source * source : sources)
                delete source;
            
            // Delete context
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(context);
        }
        alcCloseDevice(device);
    }
}

bool Listener::initialize() {
    
    // Only one context is allowed
    if (alcGetCurrentContext()) {
        LOG("Only one OpenAL context can be active at a time!");
        return false;
    }
    
    // Open device
    device = alcOpenDevice(nullptr);
    if (!device) {
        LOG("Failed to create OpenAL device");
        return false;
    }
    
    // Check EFX
    efx = alcIsExtensionPresent(device, ALC_EXT_EFX_NAME) == AL_TRUE;
    if (efx) {
        ALCint major, minor;
        alcGetIntegerv(device, ALC_EFX_MAJOR_VERSION, 1, &major);
        alcGetIntegerv(device, ALC_EFX_MINOR_VERSION, 1, &minor);
        LOG("OpenAL EFX: ", major, '.', minor);
    } else
        LOG("OpenAL EFX: <none>");
    
    // Create context
    ALint attribs[4] = {0}; 
    if (efx) {
        attribs[0] = ALC_MAX_AUXILIARY_SENDS;
        attribs[1] = 4;
    }
    context = alcCreateContext(device, attribs);
    if (!context || !alcMakeContextCurrent(context)) {
        LOG("Failed to create OpenAL context");
        if (context) {
            alcDestroyContext(context);
            context = nullptr;
        }
        alcCloseDevice(device);
        device = nullptr;
        return false;
    }
    
    // Print version
    LOG("OpenAL version: ", alGetString(AL_VERSION));
    LOG("OpenAL renderer: ", alGetString(AL_RENDERER));
    
    // Allocate sources
    uint32_t max_bindings = 256; // TODO define from arguments
    for (uint32_t i = 0; i < max_bindings; ++i) {
        ALuint handle;
        alGenSources(1, &handle);
        bindings.push_back(new Binding({handle, nullptr}));
    }
    return true;
}

void Listener::update() {

    for(Source* source : _toAdd)
    {
        sources.push_front(source);
        source->iterator = sources.begin();
    }
    _toAdd.clear();

    // Update listener location
    vec3 position(_transform.inverted().translation());
    vec3 forward(-_transform[2]);
    vec3 up(_transform[1]);

    alListener3f(AL_POSITION, position.x(), position.y(), position.z());
    ALfloat orientation[] = {forward.x(), forward.y(), forward.z(), up.x(), up.y(), up.z()};
    alListenerfv(AL_ORIENTATION, orientation);
    alListener3f(AL_VELOCITY, _velocity.x(), _velocity.y(), _velocity.z());
    
    // Update sources location
    for (Source * source : sources)
        if (source->binding) {
            // TODO do not call OpenAL if nothing changed
            vec3 position = source->position();
            alSource3f(source->binding->handle, AL_POSITION, position.x(), position.y(), position.z());
            vec3 velocity = source->velocity();
            alSource3f(source->binding->handle, AL_VELOCITY, velocity.x(), velocity.y(), velocity.z());

            if(source->getSound().sound()->isStream())
                source->getSound().sound()->update();
        }
    
    // TODO reduce CPU overload by doing these checks at a lower frequency
    
    // Check if sources have ended
    ALint state;
    for (Binding * binding : bindings)
        if (binding->source) {
            alGetSourcei(binding->handle, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING)
                binding->source->stop();
        }
}

//Sound * Listener::addSoundBuffer(resource::SoundAsset sampler) {
//    if (device) {
//        Sound * sound = new Sound(sampler, false);
//        sounds.push_front(sound);
//        return sound;
//    }
//    return nullptr;
//}

//Sound * Listener::addSoundStream(resource::SoundAsset sampler) {
//    if (device) {
//        Sound * sound = new Sound(sampler, true);
//        sounds.push_front(sound);
//        return sound;
//    }
//    return nullptr;
//}

Source * Listener::addSource(const resource::SoundAsset& sound) {
    if (device && !sound.isNull()) {
        Source * source = new Source(this);
        source->sound = sound;

        _lock.lock();
        _toAdd.push_back(source);
        _lock.unlock();

        return source;
    }
    return nullptr;
}
