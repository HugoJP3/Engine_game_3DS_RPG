#pragma once
#include <3ds.h>
#include <string>
#include <vector>
#include <map>
#include <list>

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

    Sound& getSound(const std::string& path);

    void playBGM(const Sound& sound, const std::string& path);
    void stopBGM();
    const std::string& getCurrentBGMPath() const { return currentBgmPath; }

    void playSFX(const Sound& sound, float rateMul = 1.0f);

private:
    AudioManager() = default;
    
    Sound loadWav(const std::string& path);
    void freeSound(Sound& s);

    static const int BGM_CHANNEL = 0;
    static const int SFX_START = 1;
    static const int SFX_CHANNELS = 7;

    std::string currentBgmPath;

    ndspWaveBuf bgmBuf;
    std::vector<ndspWaveBuf> sfxBufs;

    std::map<std::string, Sound> cache;
    std::list<std::string> lruOrder;

    int getFreeSFXChannel();
};