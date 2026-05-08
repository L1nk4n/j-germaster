#ifndef AUDIO_H
#define AUDIO_H

#define MINIAUDIO_IMPLEMENTATION
#include "../include/miniaudio/miniaudio.h"
#include <iostream>

namespace Audio {
  inline ma_engine engine;
  inline bool initialized = false;

  inline bool init() {
    if(ma_engine_init(NULL, &engine) != MA_SUCCESS) {
      std::cout << "Failed to init audio engine" << std::endl;
      return false;
    }
    initialized = true;
    return true;
  }

  inline void shutdown() {
    if (initialized) ma_engine_uninit(&engine);
  }

  inline void playOneShot(const char* path) {
    if (!initialized) return;
    ma_engine_play_sound(&engine, path, NULL);
  }
}

#endif
