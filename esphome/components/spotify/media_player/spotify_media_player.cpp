#include "spotify_media_player.h"

#ifdef USE_SPOTIFY

#include "esphome/core/log.h"
#include <esp_http_server.h>

namespace esphome {
namespace spotify {

static const char *const TAG = "spotify";

void SpotifyMediaPlayer::setup() {
  // Initialize audio sink
  audio_sink_ = std::make_unique<SpotifyAudioSink>(speaker_);
  audio_sink_->setParams(44100, 2, 16);
  
  // Set up HTTP server for Spotify Connect authentication
  setup_http_server();
}

void SpotifyMediaPlayer::loop() {
  if (context_ != nullptr) {
    context_->session->handlePacket();
  }
}

void SpotifyMediaPlayer::control(const media_player::MediaPlayerCall &call) {
  if (spirc_handler_ == nullptr)
    return;
    
  if (call.get_command().has_value()) {
    switch (call.get_command().value()) {
      case media_player::MEDIA_PLAYER_COMMAND_PLAY:
        spirc_handler_->play();
        break;
      case media_player::MEDIA_PLAYER_COMMAND_PAUSE:
        spirc_handler_->pause();
        break;
      case media_player::MEDIA_PLAYER_COMMAND_STOP:
        spirc_handler_->stop();
        break;
      case media_player::MEDIA_PLAYER_COMMAND_NEXT:
        spirc_handler_->next();
        break;
      case media_player::MEDIA_PLAYER_COMMAND_PREVIOUS:
        spirc_handler_->previous();
        break;
      default:
        break;
    }
  }
}

media_player::MediaPlayerTraits SpotifyMediaPlayer::get_traits() {
  auto traits = media_player::MediaPlayerTraits();
  traits.set_supports_pause(true);
  traits.set_supports_stop(true);
  traits.set_supports_next(true);
  traits.set_supports_previous(true);
  traits.set_supports_volume(true);
  return traits;
}

void SpotifyMediaPlayer::handle_playback_event(std::unique_ptr<cspot::SpircHandler::Event> event) {
  switch (event->eventType) {
    case cspot::SpircHandler::EventType::TRACK_INFO:
      if (auto track_info = std::get<cspot::TrackInfo>(event->data)) {
        this->state = media_player::MEDIA_PLAYER_STATE_PLAYING;
        this->publish_state();
      }
      break;
    case cspot::SpircHandler::EventType::PLAY_PAUSE:
      this->state = std::get<bool>(event->data) ? media_player::MEDIA_PLAYER_STATE_PLAYING 
                                               : media_player::MEDIA_PLAYER_STATE_PAUSED;
      this->publish_state();
      break;
  }
}

}  // namespace spotify
}  // namespace esphome

#endif  // USE_SPOTIFY
