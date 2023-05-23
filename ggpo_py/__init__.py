from enum import Enum
from ggpo_py.ggpoplayer import *
from ggpo_py.ggpo_py import *


class GGPOErrorCode(Enum):
    """
Enum holding all the error codes defined in
src/include/ggponet.h
"""

    GGPO_ERRORCODE_SUCCESS = 0
    GGPO_ERRORCODE_GENERAL_FAILURE = -1
    GGPO_ERRORCODE_INVALID_SESSION = 1
    GGPO_ERRORCODE_INVALID_PLAYER_HANDLE = 2
    GGPO_ERRORCODE_PLAYER_OUT_OF_RANGE = 3
    GGPO_ERRORCODE_PREDICTION_THRESHOLD = 4
    GGPO_ERRORCODE_UNSUPPORTED = 5
    GGPO_ERRORCODE_NOT_SYNCHRONIZED = 6
    GGPO_ERRORCODE_IN_ROLLBACK = 7
    GGPO_ERRORCODE_INPUT_DROPPED = 8
    GGPO_ERRORCODE_PLAYER_DISCONNECTED = 9
    GGPO_ERRORCODE_TOO_MANY_SPECTATORS = 10
    GGPO_ERRORCODE_INVALID_REQUEST = 11


class GGPOEventCode(Enum):
    """
Defined in src/include/ggponet.h

/*
 * The GGPOEventCode enumeration describes what type of event just happened.
 *
 * GGPO_EVENTCODE_CONNECTED_TO_PEER - Handshake with the game running on the
 * other side of the network has been completed.
 *
 * GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER - Beginning the synchronization
 * process with the client on the other end of the networking.  The count
 * and total fields in the u.synchronizing struct of the GGPOEvent
 * object indicate progress.
 *
 * GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER - The synchronziation with this
 * peer has finished.
 *
 * GGPO_EVENTCODE_RUNNING - All the clients have synchronized.  You may begin
 * sending inputs with ggpo_synchronize_inputs.
 *
 * GGPO_EVENTCODE_DISCONNECTED_FROM_PEER - The network connection on
 * the other end of the network has closed.
 *
 * GGPO_EVENTCODE_TIMESYNC - The time synchronziation code has determined
 * that this client is too far ahead of the other one and should slow
 * down to ensure fairness.  The u.timesync.frames_ahead parameter in
 * the GGPOEvent object indicates how many frames the client is.
 *
 */
 """
    GGPO_EVENTCODE_CONNECTED_TO_PEER = 1000
    GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER = 1001
    GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER = 1002
    GGPO_EVENTCODE_RUNNING = 1003
    GGPO_EVENTCODE_DISCONNECTED_FROM_PEER = 1004
    GGPO_EVENTCODE_TIMESYNC = 1005
    GGPO_EVENTCODE_CONNECTION_INTERRUPTED = 1006
    GGPO_EVENTCODE_CONNECTION_RESUMED = 1007


class GGPOEvent:
    code: GGPOEventCode
    player: GGPOPlayer
    count: int
    total: int
    frames_ahead: int
    disconnect_timeout: int

