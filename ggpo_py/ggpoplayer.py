from enum import Enum


class GGPOPlayerType(Enum):
    GGPO_PLAYERTYPE_LOCAL = 0
    GGPO_PLAYERTYPE_REMOTE = 1
    # GGPO_PLAYERTYPE_SPECTATOR = 2 # NOT IMPLEMENTED


class GGPOPlayer:
    def __init__(self, player_idx, player_type: GGPOPlayerType):
        self.player_type = player_type
        self.player_nr = player_idx
        self.player_ip = ""
        self.port = 0