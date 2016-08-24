
#include "Sampler.hpp"
#include "core/Logger.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#ifndef NO_OGG_VORBIS
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>
#endif

Sampler::Sampler() : reader(nullptr) {}

Sampler::~Sampler() {
    delete reader;
}

bool Sampler::load(std::string const & path, Conversion conversion) {
    // TODO improve this based on extension
    return loadWav(path, conversion) || loadOgg(path, conversion);
}

bool Sampler::loadWav(std::string const & path, Conversion conversion) {
    delete reader;
    reader = createResampler(createWav(path), conversion);
    return reader;
}

bool Sampler::loadOgg(std::string const & path, Conversion conversion) {
    delete reader;
    reader = createResampler(createOgg(path), conversion);
    return reader;
}

int Sampler::getFormat() const {
    if (!reader)
        return AL_NONE;
    if (reader->channels == 2)
        return reader->bits == 16 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
    return reader->bits == 16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
}

uint32_t Sampler::getChannels() const {
    return reader ? reader->channels : 0;
}

uint32_t Sampler::getBits() const {
    return reader ? reader->bits : 0;
}

uint32_t Sampler::getBytesPerSample() const {
    return reader ? reader->channels * reader->bits / 8 : 0;
}

uint32_t Sampler::getFrequency() const {
    return reader ? reader->frequency : 0;
}

uint32_t Sampler::getSize() const {
    return reader ? reader->size : 0;
}

uint32_t Sampler::getBytes() const {
    return reader ? reader->size * reader->channels * reader->bits / 8 : 0;
}

float Sampler::getDuration() const {
    return reader ? (float)reader->size / (float)(reader->frequency) : 0.0f;
}

uint32_t Sampler::getOffset() const {
    return reader ? reader->offset : 0;
}

void Sampler::rewind() {
    if (reader)
        reader->rewind();
}

uint32_t Sampler::read(void * buffer, uint32_t samples) {
    return reader ? reader->read(buffer, samples) : 0;
}

Sampler::Reader * Sampler::createWav(std::string const & path) {
    // http://soundfile.sapp.org/doc/WaveFormat/
    // http://www.topherlee.com/software/pcm-tut-wavformat.html
    // http://unusedino.de/ec64/technical/formats/wav.html
    
    struct WavReader : Reader {
        FILE * file;
        uint32_t start;
        
        WavReader(std::string const & path) {
            
            // Open file
            file = fopen(path.c_str(), "rb");
            if (!file)
                return;
            
            // Read header
            char header[36];
            if (fread(header, sizeof(header), 1, file) != 1) {
                fclose(file);
                file = nullptr;
                return;
            }
            
            // Check PCM header
            if (header[0] != 'R' || header[1] != 'I' || header[2] != 'F' || header[3] != 'F' ||
                    header[8] != 'W' || header[9] != 'A' || header[10] != 'V' || header[11] != 'E' ||
                    header[12] != 'f' || header[13] != 'm' || header[14] != 't' || header[15] != ' ' ||
                    *(uint32_t *)(header + 16) != 16 || *(uint16_t *)(header + 20) != 1) {
                LOG(path, " is not a valid RIFF WAVE PCM");
                fclose(file);
                file = nullptr;
                return;
            }
            
            // Check format
            bits = *(uint16_t *)(header + 34);
            channels = *(uint16_t *)(header + 22);
            if ((bits != 8 && bits != 16) || (channels != 1 && channels != 2)) {
                LOG(path, " invalid WAV format, only mono/stereo 8/16 bits is supported");
                fclose(file);
                file = nullptr;
                return;
            }
            frequency = *(uint32_t *)(header + 24);
            
            // Search data chunk
            start = 0;
            char chunk[8];
            while (start == 0) {
                if (fread(chunk, sizeof(chunk), 1, file) != 1)
                    break;
                size = *(uint32_t*)(chunk + 4);
                if (chunk[0] == 'd' && chunk[1] == 'a' && chunk[2] == 't' && chunk[3] == 'a') {
                    start = ftell(file);
                    break;
                } else
                    fseek(file, size, SEEK_CUR);
            }
            if (start == 0) {
                LOG("wav data chunk not found");
                fclose(file);
                file = nullptr;
                return;
            }
            offset = 0;
            size /= channels * bits / 8;
        }
        
        ~WavReader() {
            if (file)
                fclose(file);
        }
        
        void rewind() {
            fseek(file, start, SEEK_SET);
            offset = 0;
        }
        
        uint32_t read(void * buffer, uint32_t samples) {
            samples = fread(buffer, channels * bits / 8, std::min(samples, size - offset), file);
            offset += samples;
            return samples;
        }
        
    };
    WavReader * reader = new WavReader(path);
    if (reader->file)
        return reader;
    delete reader;
    return nullptr;
}

Sampler::Reader * Sampler::createOgg(std::string const & path) {
    // https://xiph.org/vorbis/doc/vorbisfile/example.html
    // http://www.gamedev.net/page/resources/_/technical/game-programming/introduction-to-ogg-vorbis-r2031
    // https://xiph.org/vorbis/doc/vorbisfile/reference.html

#ifndef NO_OGG_VORBIS
    struct OggReader : Reader {
        FILE * file;
        OggVorbis_File vorbis;
        
        OggReader(std::string const & path) {
            
            // Open file
            file = fopen(path.c_str(), "rb");
            if (!file)
                return;
            
            // Create Vorbis object
            if (ov_open(file, &vorbis, NULL, 0) < 0) {
                ov_clear(&vorbis);
                fclose(file);
                file = nullptr;
                return;
            }
            
            // Read infos
            vorbis_info * info = ov_info(&vorbis, -1);
            channels = info->channels;
            bits = 16;
            size = ov_pcm_total(&vorbis, -1);
            frequency = info->rate;
            offset = 0;
        }
        
        ~OggReader() {
            if (file) {
                ov_clear(&vorbis);
                fclose(file);
            }
        }
        
        void rewind() {
            ov_raw_seek(&vorbis, 0);
            offset = 0;
        }
        
        uint32_t read(void * buffer, uint32_t samples) {
            int sample_size = channels * 2;
            char * pointer = (char *)buffer;
            int size = samples * sample_size;
            int bitStream;
            while (true) {
                int s = ov_read(&vorbis, pointer, size, 0, 2, 1, &bitStream);
                size -= s;
                pointer += s;
                if (s == 0 || size <= 0)
                    break;
            }
            return (pointer - (char *)buffer) / sample_size;
        }
        
    };
    OggReader * reader = new OggReader(path);
    if (reader->file)
        return reader;
    delete reader;
#endif
    return nullptr;
}

#define RESAMPLER_SIZE 1024

Sampler::Reader * Sampler::createResampler(Reader * reader, Conversion conversion) {
    // http://dsp.stackexchange.com/questions/3581/algorithms-to-mix-audio-signals-without-clipping
    
    struct Resampler : Reader {
        Reader * reader;
        Conversion conversion;
        char tmp[RESAMPLER_SIZE * 4];
        
        Resampler(Reader * reader, Conversion conversion) : reader(reader), conversion(conversion) {
            channels = 1;
            bits = reader->bits;
            frequency = reader->frequency;
            size = reader->size;
            offset = reader->offset;
        }
        
        ~Resampler() {
            delete reader;
        }
        
        void rewind() {
            reader->rewind();
        }
        
        uint32_t read(void * buffer, uint32_t samples) {
            
            // Split large read into smaller ones
            uint32_t total = 0;
            while (samples > RESAMPLER_SIZE) {
                uint32_t subcount = read(buffer, RESAMPLER_SIZE);
                total += subcount;
                if (subcount < RESAMPLER_SIZE)
                    return total;
                buffer = ((char *)buffer) + RESAMPLER_SIZE * bits / 8;
                samples -= RESAMPLER_SIZE;
            }
            
            // Read a chunk
            uint32_t count = reader->read(tmp, samples);
            total += count;
            
            // Convert to mono
            switch (conversion) {
                case LEFT:
                    if (bits == 8) {
                        for (uint32_t i = 0; i < count; ++i)
                            ((int8_t *)buffer)[i] = ((int8_t *)tmp)[2 * i];
                    } else {
                        for (uint32_t i = 0; i < count; ++i)
                            ((int16_t *)buffer)[i] = ((int16_t *)tmp)[2 * i];
                    }
                    break;
                case RIGHT:
                    if (bits == 8) {
                        for (uint32_t i = 0; i < count; ++i)
                            ((int8_t *)buffer)[i] = ((int8_t *)tmp)[2 * i + 1];
                    } else {
                        for (uint32_t i = 0; i < count; ++i)
                            ((int16_t *)buffer)[i] = ((int16_t *)tmp)[2 * i + 1];
                    }
                    break;
                default:
                    // TODO this mixer could be improved :/
                    if (bits == 8) {
                        for (uint32_t i = 0; i < count; ++i) {
                            float left = ((int8_t *)tmp)[2 * i] / 127.0f;
                            float right = ((int8_t *)tmp)[2 * i + 1] / 127.0f;
                            float combined = std::tanh(left + right);
                            ((int8_t *)buffer)[i] = (int8_t)(combined * 127.0f);
                        }
                    } else {
                        for (uint32_t i = 0; i < count; ++i) {
                            float left = ((int16_t *)tmp)[2 * i] / 32767.0f;
                            float right = ((int16_t *)tmp)[2 * i + 1] / 32767.0f;
                            float combined = std::tanh(left + right);
                            ((int16_t *)buffer)[i] = (int16_t)(combined * 32767.0f);
                        }
                    }
                    break;
            }
            return total;
        }
        
    };
    if (conversion == NONE || reader == nullptr || reader->channels != 2)
        return reader;
    return new Resampler(reader, conversion);
}
