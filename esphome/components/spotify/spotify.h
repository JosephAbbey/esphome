#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/speaker/speaker.h"
#include "esphome/components/button/button.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/image/image.h"
#include "spotify_audio_sink.h"

#include <CSpotContext.h>
#include <LoginBlob.h>
#include <SpircHandler.h>
#include <memory>

namespace esphome {
namespace spotify {

class SpotifyComponent : public Component {
 public:
  void set_device_name(const std::string &device_name) { device_name_ = device_name; }
  void set_speaker(speaker::Speaker *speaker) { speaker_ = speaker; }
  
  void set_play_button(button::Button *play_button) { play_button_ = play_button; }
  void set_pause_button(button::Button *pause_button) { pause_button_ = pause_button; }
  void set_next_button(button::Button *next_button) { next_button_ = next_button; }
  void set_previous_button(button::Button *prev_button) { prev_button_ = prev_button; }
  
  void set_track_name_sensor(text_sensor::TextSensor *sensor) { track_name_sensor_ = sensor; }
  void set_artist_name_sensor(text_sensor::TextSensor *sensor) { artist_name_sensor_ = sensor; }
  void set_album_name_sensor(text_sensor::TextSensor *sensor) { album_name_sensor_ = sensor; }
  void set_album_art_image(image::Image *image) { album_art_image_ = image; }
  
  void setup() override;
  void loop() override;

 protected:
  void setup_http_server();
  void handle_auth_success();
  void handle_playback_event(std::unique_ptr<cspot::SpircHandler::Event> event);
  void update_track_info(const cspot::TrackInfo &track);
  void update_album_art(const std::string &url);

  std::string device_name_;
  speaker::Speaker *speaker_{nullptr};
  std::unique_ptr<SpotifyAudioSink> audio_sink_;
  std::shared_ptr<cspot::LoginBlob> login_blob_;
  std::shared_ptr<cspot::Context> context_;
  std::shared_ptr<cspot::SpircHandler> spirc_handler_;

  // Controls
  button::Button *play_button_{nullptr};
  button::Button *pause_button_{nullptr};
  button::Button *next_button_{nullptr};
  button::Button *prev_button_{nullptr};

  // Sensors
  text_sensor::TextSensor *track_name_sensor_{nullptr};
  text_sensor::TextSensor *artist_name_sensor_{nullptr};
  text_sensor::TextSensor *album_name_sensor_{nullptr};
  image::Image *album_art_image_{nullptr};
};

}  // namespace spotify
}  // namespace esphome
