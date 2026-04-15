#include "AudioManager.hpp"
#include <cstdio>
#include <cstring>

void AudioManager::init() {
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);

    sfxBufs.resize(SFX_CHANNELS);
    for (auto& b : sfxBufs) memset(&b, 0, sizeof(ndspWaveBuf));
}

AudioManager& AudioManager::get() {
    static AudioManager instance;
    return instance;
}


void AudioManager::exit() {
    for (auto& pair : cache) {
        freeSound(pair.second);
    }
    cache.clear();

    ndspExit();
}

Sound& AudioManager::getSound(const std::string& path) {
    // Si ya está, devolver + LRU
    if (cache.count(path)) {
        lruOrder.remove(path);
        lruOrder.push_back(path);
        return cache[path];
    }

    // Intenta cargar
    Sound s = loadWav(path);
    
    // Si no hay memoria → liberar hasta que funcione
    while (!s.data) {

        if (lruOrder.empty()) {
            // No queda nada que liberar → imposible cargar
            static Sound empty;
            return empty;
        }

        // Liberar el más antiguo
        std::string oldest = lruOrder.front();
        if (oldest == currentBgmPath) {
            lruOrder.pop_front();
            lruOrder.push_back(oldest);
            continue;
        }

        lruOrder.pop_front();
        freeSound(cache[oldest]);
        cache.erase(oldest);

        // Reintentar
        s = loadWav(path);
    }

    // Guardar en caché
    cache[path] = s;
    lruOrder.push_back(path);

    return cache[path];
}

void AudioManager::update() {
    // Limpia buffers SFX terminados
    for (int i = 0; i < SFX_CHANNELS; i++) {
        if (sfxBufs[i].status == NDSP_WBUF_DONE) {
            ndspChnWaveBufClear(SFX_START + i);
        }
    }
}

Sound AudioManager::loadWav(const std::string& path) {
    Sound s;

    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return s;

    char riff[4];
    fread(riff, 1, 4, f);
    if (memcmp(riff, "RIFF", 4) != 0) {
        fclose(f);
        return s;
    }

    fseek(f, 22, SEEK_SET);

    u16 channels;
    fread(&channels, sizeof(u16), 1, f);
    fread(&s.sampleRate, sizeof(u32), 1, f);

    fseek(f, 34, SEEK_SET);

    u16 bitsPerSample;
    fread(&bitsPerSample, sizeof(u16), 1, f);

    char chunk[4];
    u32 chunkSize;

    while (true) {
        if (fread(chunk, 1, 4, f) != 4) {
            fclose(f);
            return s;
        }

        fread(&chunkSize, 4, 1, f);

        if (memcmp(chunk, "data", 4) == 0) break;

        fseek(f, chunkSize, SEEK_CUR);
    }

    s.size = chunkSize;
    s.data = linearAlloc(s.size);
    if (!s.data) {
        fclose(f);
        return s;
    }

    fread(s.data, 1, s.size, f);
    fclose(f);

    s.stereo = (channels == 2);
    s.nsamples = s.size / (bitsPerSample / 8) / channels;

    return s;
}


void AudioManager::playBGM(const Sound& s, const std::string& path) {
    if (!s.data || s.size == 0) return;
    currentBgmPath = path;
    
    ndspChnReset(BGM_CHANNEL);

    ndspChnSetInterp(BGM_CHANNEL, NDSP_INTERP_LINEAR);
    ndspChnSetRate(BGM_CHANNEL, s.sampleRate);
    ndspChnSetFormat(BGM_CHANNEL,
        s.stereo ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);

    memset(&bgmBuf, 0, sizeof(ndspWaveBuf));
    bgmBuf.data_vaddr = s.data;
    bgmBuf.nsamples = s.nsamples;
    bgmBuf.looping = true;

    DSP_FlushDataCache(s.data, s.size);

    ndspChnWaveBufAdd(BGM_CHANNEL, &bgmBuf);
}


void AudioManager::stopBGM() {
    ndspChnReset(BGM_CHANNEL);
}

int AudioManager::getFreeSFXChannel() {
    for (int i = 0; i < SFX_CHANNELS; i++) {
        if (sfxBufs[i].status != NDSP_WBUF_PLAYING) {
            return i;
        }
    }
    return -1;
}

void AudioManager::playSFX(const Sound& s) {
    if (!s.data || s.size == 0) return;

    int idx = getFreeSFXChannel();
    if (idx == -1) return;

    int ch = SFX_START + idx;

    ndspChnReset(ch);

    ndspChnSetInterp(ch, NDSP_INTERP_LINEAR);
    ndspChnSetRate(ch, s.sampleRate);
    ndspChnSetFormat(ch,
        s.stereo ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16);

    memset(&sfxBufs[idx], 0, sizeof(ndspWaveBuf));
    sfxBufs[idx].data_vaddr = s.data;
    sfxBufs[idx].nsamples = s.nsamples;

    DSP_FlushDataCache(s.data, s.size);

    ndspChnWaveBufAdd(ch, &sfxBufs[idx]);
}

void AudioManager::freeSound(Sound& s) {
    if (s.data) {
        linearFree(s.data);
        s.data = nullptr;
    }
}