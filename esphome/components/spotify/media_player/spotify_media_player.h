#pragma once

#ifdef USE_SPOTIFY

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/media_player/media_player.h"
#include "esphome/components/speaker/speaker.h"

#include <BufferedAudioSink.h>

#include <CSpotContext.h>
#include <LoginBlob.h>
#include <SpircHandler.h>
#include <memory>

namespace esphome {
namespace spotify {

class SpotifyMediaPlayer : public media_player::MediaPlayer, public Component {
 public:
  void set_device_name(const std::string &device_name) { device_name_ = device_name; }
  void set_speaker(speaker::Speaker *speaker) { speaker_ = speaker; }
  
  void setup() override;
  void loop() override;
  
  // Media player control methods
  void control(const media_player::MediaPlayerCall &call) override;
  media_player::MediaPlayerTraits get_traits() override;

 protected:
  void handle_playback_event(std::unique_ptr<cspot::SpircHandler::Event> event);
  void setup_http_server();
  void handle_auth_success();
  
  std::string device_name_;
  speaker::Speaker *speaker_{nullptr};
  std::unique_ptr<SpotifyAudioSink> audio_sink_;
  std::shared_ptr<cspot::LoginBlob> login_blob_;
  std::shared_ptr<cspot::Context> context_;
  std::shared_ptr<cspot::SpircHandler> spirc_handler_;
};

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

#endif  // USE_SPOTIFY
