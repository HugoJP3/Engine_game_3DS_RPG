#pragma once
#include <3ds.h>
#include <string>
#include <vector>

struct Sound {
    void* data = nullptr;
    u32 size = 0;
    u32 sampleRate = 0;
    bool stereo = false;
    u32 nsamples = 0;
};

class AudioManager {
public:
    static AudioManager& get(); 
    
    void init();
    void exit();
    void update();

    Sound loadWav(const std::string& path);

    void playBGM(const Sound& sound);
    void stopBGM();
    void playSFX(const Sound& sound);

private:
    AudioManager() = default;
    
    static const int BGM_CHANNEL = 0;
    static const int SFX_START = 1;
    static const int SFX_CHANNELS = 7;

    ndspWaveBuf bgmBuf;
    std::vector<ndspWaveBuf> sfxBufs;

    int getFreeSFXChannel();
};