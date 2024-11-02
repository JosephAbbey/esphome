import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME
from esphome.components import speaker, media_player
from esphome.core import CORE
from esphome.components.spotify import download_cspot

DEPENDENCIES = ["network", "speaker", "mdns"]
AUTO_LOAD = ["json"]

CODEOWNERS = ["@josephabbey"]

spotify_ns = cg.esphome_ns.namespace("spotify")
SpotifyMediaPlayer = spotify_ns.class_(
    "SpotifyMediaPlayer", media_player.MediaPlayer, cg.Component
)

CONF_DEVICE_NAME = "device_name"
CONF_SPEAKER_ID = "speaker_id"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SpotifyMediaPlayer),
            cv.Required(CONF_NAME): cv.string,
            cv.Required(CONF_DEVICE_NAME): cv.string,
            cv.Required(CONF_SPEAKER_ID): cv.use_id(speaker.Speaker),
        }
    )
    .extend(media_player.MEDIA_PLAYER_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    cspot_dir = download_cspot()

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await media_player.register_media_player(var, config)

    cg.add(var.set_device_name(config[CONF_DEVICE_NAME]))

    speaker_component = await cg.get_variable(config[CONF_SPEAKER_ID])
    cg.add(var.set_speaker(speaker_component))

    # Add build flags and includes for cspot
    cg.add_build_flag("-DUSE_SPOTIFY")
    cg.add_build_flag(f"-I{cspot_dir}/cspot/include")
    cg.add_build_flag(f"-I{cspot_dir}/cspot/src")
    cg.add_build_flag(f"-I{cspot_dir}/cspot/bell/main/audio-sinks/include")
    cg.add_build_flag(f"-I{cspot_dir}/cspot/bell/main/audio-sinks/esp")
