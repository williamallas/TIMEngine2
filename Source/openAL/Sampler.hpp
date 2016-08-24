
#ifndef GLOW_SAMPLER_HPP
#define GLOW_SAMPLER_HPP

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>

#include <string>

class Sampler {
public:
    
    enum Conversion {
        NONE,
        LEFT,
        RIGHT,
        SUM
        // Note: forcing stereo is useless
    };
    
    Sampler();
    ~Sampler();
    
    Sampler(Sampler const &) = delete;
    Sampler & operator=(Sampler const &) = delete;
    
    bool load(std::string const & path, Conversion conversion = NONE);
    bool loadWav(std::string const & path, Conversion conversion = NONE);
    bool loadOgg(std::string const & path, Conversion conversion = NONE);
    
    int getFormat() const;
    uint32_t getChannels() const;
    uint32_t getBits() const;
    uint32_t getBytesPerSample() const;
    uint32_t getFrequency() const;
    uint32_t getSize() const;
    uint32_t getBytes() const;
    float getDuration() const;
    uint32_t getOffset() const;
    
    void rewind();
    uint32_t read(void * buffer, uint32_t samples);
    
private:

    struct Reader {
        uint32_t channels;
        uint32_t bits;
        uint32_t frequency;
        uint32_t size;
        uint32_t offset;
        virtual ~Reader() {}
        virtual void rewind() = 0;
        virtual uint32_t read(void * buffer, uint32_t samples) = 0;
    };
    Reader * reader;
    
    Reader * createWav(std::string const & path);
    Reader * createOgg(std::string const & path);
    
    Reader * createResampler(Reader * reader, Conversion conversion);
    
};

#endif
