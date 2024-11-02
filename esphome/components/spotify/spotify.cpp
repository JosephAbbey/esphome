#include "spotify.h"

#ifdef USE_SPOTIFY

#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include <esp_http_server.h>

namespace esphome {
namespace spotify {

static const char *const TAG = "spotify";

void SpotifyComponent::setup_http_server() {
  httpd_handle_t server = nullptr;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 8080;
  
  ESP_ERROR_CHECK(httpd_start(&server, &config));

  // Register endpoints for Spotify Connect authentication
  httpd_uri_t spotify_info_get = {
    .uri = "/spotify_info",
    .method = HTTP_GET,
    .handler = [](httpd_req_t *req) -> esp_err_t {
      auto *spotify = static_cast<SpotifyComponent *>(req->user_ctx);
      std::string response = spotify->login_blob_->buildZeroconfInfo();
      httpd_resp_send(req, response.c_str(), response.length());
      return ESP_OK;
    },
    .user_ctx = this
  };
  httpd_register_uri_handler(server, &spotify_info_get);

  // Add POST handler for authentication data
  httpd_uri_t spotify_info_post = {
    .uri = "/spotify_info",
    .method = HTTP_POST,
    .handler = [](httpd_req_t *req) -> esp_err_t {
      auto *spotify = static_cast<SpotifyComponent *>(req->user_ctx);
      
      // Handle authentication data
      char content[512];
      size_t recv_size = MIN(req->content_len, sizeof(content));
      int ret = httpd_req_recv(req, content, recv_size);
      
      if (ret > 0) {
        // Parse and handle authentication data
        spotify->handle_auth_success();
      }
      
      const char *response = "{\"status\":101,\"spotifyError\":0,\"statusString\":\"ERROR-OK\"}";
      httpd_resp_send(req, response, strlen(response));
      return ESP_OK;
    },
    .user_ctx = this
  };
  httpd_register_uri_handler(server, &spotify_info_post);
}

void SpotifyComponent::handle_auth_success() {
  context_ = cspot::Context::createFromBlob(login_blob_);
  context_->session->connectWithRandomAp();
  auto token = context_->session->authenticate(login_blob_);

  if (token.size() > 0) {
    context_->session->startTask();
    spirc_handler_ = std::make_shared<cspot::SpircHandler>(context_);
    spirc_handler_->subscribeToMercury();

    // Set up event handlers for playback control and metadata
    spirc_handler_->setEventHandler([this](std::unique_ptr<cspot::SpircHandler::Event> event) {
      this->handle_playback_event(std::move(event));
    });

    // Set up button handlers
    if (play_button_ != nullptr) {
      play_button_->add_on_press_callback([this]() {
        if (spirc_handler_ != nullptr)
          spirc_handler_->play();
      });
    }
    // ... similar for other buttons
  }
}

void SpotifyComponent::handle_playback_event(std::unique_ptr<cspot::SpircHandler::Event> event) {
  switch (event->eventType) {
    case cspot::SpircHandler::EventType::TRACK_INFO:
      if (auto track_info = std::get<cspot::TrackInfo>(event->data)) {
        update_track_info(track_info);
      }
      break;
    case cspot::SpircHandler::EventType::PLAY_PAUSE:
      // Update play/pause state
      break;
    // ... handle other events
  }
}

void SpotifyComponent::update_track_info(const cspot::TrackInfo &track) {
  if (track_name_sensor_ != nullptr)
    track_name_sensor_->publish_state(track.name);
  if (artist_name_sensor_ != nullptr)
    artist_name_sensor_->publish_state(track.artist_name);
  if (album_name_sensor_ != nullptr)
    album_name_sensor_->publish_state(track.album_name);
  if (album_art_image_ != nullptr && !track.cover_url.empty())
    update_album_art(track.cover_url);
}

void SpotifyComponent::update_album_art(const std::string &url) {
  // Download and update album art
  // This should be done in a separate task to avoid blocking
  // Implementation depends on ESPHome's HTTP client and image component
}

}  // namespace spotify
}  // namespace esphome

#endif  // USE_SPOTIFY
