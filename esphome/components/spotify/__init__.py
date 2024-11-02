import os
import subprocess

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.core import CORE
from esphome.components import speaker

DEPENDENCIES = ["network", "speaker", "mdns"]
AUTO_LOAD = ["json"]

CODEOWNERS = ["@josephabbey"]

spotify_ns = cg.esphome_ns.namespace("spotify")
SpotifyComponent = spotify_ns.class_("SpotifyComponent", cg.Component)

CONF_DEVICE_NAME = "device_name"
CONF_SPEAKER_ID = "speaker_id"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SpotifyComponent),
        cv.Required(CONF_DEVICE_NAME): cv.string,
        cv.Required(CONF_SPEAKER_ID): cv.use_id(speaker.Speaker),
    }
).extend(cv.COMPONENT_SCHEMA)


def download_cspot():
    cspot_dir = os.path.join(CORE.config_dir, "components", "spotify", "cspot")
    if not os.path.exists(cspot_dir):
        subprocess.run(
            [
                "git",
                "clone",
                "--recursive",
                "https://github.com/feelfreelinux/cspot.git",
                cspot_dir,
            ],
            check=True,
        )
    return cspot_dir


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_device_name(config[CONF_DEVICE_NAME]))

    # Link the speaker
    speaker_component = await cg.get_variable(config[speaker.CONF_SPEAKER_ID])
    cg.add(var.set_speaker(speaker_component))

    # Add build flags and includes for cspot
    cg.add_build_flag("-DUSE_SPOTIFY")
    cg.add_build_flag("-I$PROJECT_DIR/components/spotify/cspot/src")

    # Add source files from cspot
    cg.add_source_files(
        "components/spotify/cspot/src/*.cpp", "components/spotify/cspot/src/*.c"
    )
