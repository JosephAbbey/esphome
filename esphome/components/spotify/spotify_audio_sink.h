#pragma once

#include "esphome/components/speaker/speaker.h"
#include "esphome/core/log.h"
#include "cspot/cspot/bell/main/audio-sinks/include/BufferedAudioSink.h"

namespace esphome {
namespace spotify {

class SpotifyAudioSink : public BufferedAudioSink {
 public:
  explicit SpotifyAudioSink(speaker::Speaker *speaker) : speaker_(speaker) {
    // Disable software volume control since we'll handle it through ESPHome
    this->softwareVolumeControl = false;
  }

  void volumeChanged(uint16_t volume) override {
    if (speaker_ != nullptr) {
      // Convert cspot volume (0-65535) to ESPHome volume (0.0-1.0)
      float normalized_volume = static_cast<float>(volume) / 65535.0f;
      speaker_->set_volume(normalized_volume);
    }
  }

 protected:
  void feedPCMFrames(std::vector<uint8_t> &data) override {
    if (speaker_ != nullptr) {
      // cspot provides 16-bit stereo PCM at 44100Hz
      speaker_->play(data.data(), data.size());
    }
  }

  speaker::Speaker *speaker_;
};

}  // namespace spotify
}  // namespace esphome
