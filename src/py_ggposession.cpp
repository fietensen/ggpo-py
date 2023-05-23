#include "py_ggposession.h"
#include <iostream>

PyGGPOSession *ggpo_session = nullptr;

/*
 * Simple checksum function stolen from wikipedia:
 *
 *   http://en.wikipedia.org/wiki/Fletcher%27s_checksum
 */

int
fletcher32_checksum(short *data, size_t len)
{
    int sum1 = 0xffff, sum2 = 0xffff;

    while (len) {
        size_t tlen = len > 360 ? 360 : len;
        len -= tlen;
        do {
            sum1 += *data++;
            sum2 += sum1;
        } while (--tlen);
        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    /* Second reduction step to reduce sums to 16 bits */
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    return sum2 << 16 | sum1;
}



PyGGPOSession::PyGGPOSession(const char *game_title, unsigned short game_port, int n_players, py::object game_state) {
    if (n_players > GGPO_MAX_PLAYERS)
        throw std::runtime_error("Maximum number of Players exceeded: " + std::to_string(GGPO_MAX_PLAYERS));

    _game_title = game_title;
    _game_players = n_players;
    _game_port = game_port;
    _game_state = game_state;

    _ggpo_players = new GGPOPlayer[n_players];
    _ggpo_playerhandles = new GGPOPlayerHandle[n_players];
    _py_players = new py::object[n_players];

    /*_ggpo_callbacks->save_game_state = [this] (unsigned char **buffer, int *len, int *checksum, int _) {
            return _cbk_save_game_state(buffer, len, checksum, _);};
    */
    _ggpo_callbacks.save_game_state = _cbk_save_game_state;
    _ggpo_callbacks.load_game_state = _cbk_load_game_state;
    _ggpo_callbacks.free_buffer = _cbk_free_buffer;
    _ggpo_callbacks.advance_frame = _cbk_advance_frame;
    _ggpo_callbacks.on_event = _cbk_on_event;
    _ggpo_callbacks.begin_game = _cbk_begin_game;
    _ggpo_callbacks.log_game_state = _cbk_log_game_state;

    ggpo_session = this;;
}

py::object PyGGPOSession::add_player(py::object player)
{
    int player_idx = py::cast<int>(player.attr("player_nr"));

    py::type PyGGPOPlayer = py::type(py::module_::import("ggpo_py").attr("GGPOPlayer"));
    if (!py::isinstance(player, PyGGPOPlayer))
        throw std::runtime_error("Argument of type GGPOPlayer expected.");
    if (player_idx>this->_game_players)
        throw std::runtime_error("Index out of range.");

    GGPOPlayerType player_type = (GGPOPlayerType)py::cast<int>(player.attr("player_type").attr("value"));
    std::string ip_address = player.attr("player_ip").str();
    if (ip_address.length() > 32)
        throw std::runtime_error("Maximum IP-Address length exceeded.");

    this->_py_players[player_idx] = player;
    this->_ggpo_players[player_idx].type = player_type;
    this->_ggpo_players[player_idx].player_num = player_idx+1;
    this->_ggpo_players[player_idx].size = sizeof(GGPOPlayer);

    switch (player_type) {
        case GGPO_PLAYERTYPE_LOCAL:
            break;
        case GGPO_PLAYERTYPE_REMOTE:
            strcpy(this->_ggpo_players[player_idx].u.remote.ip_address, ip_address.c_str());
            this->_ggpo_players[player_idx].u.remote.port = (unsigned short)py::int_(player.attr("port"));
            break;
        case GGPO_PLAYERTYPE_SPECTATOR:
            break; // Implementing this later
    }

    int errorcode = (int)ggpo_add_player(this->_ggpo, &this->_ggpo_players[player_idx], &this->_ggpo_playerhandles[player_idx]);

    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);
}

bool __cdecl _cbk_begin_game(const char*)
{
    return true;
}

bool __cdecl _cbk_advance_frame(int)
{
    int *inputs = new int[ggpo_session->_game_players];
    int disconnect_flags;

    ggpo_synchronize_input(ggpo_session->_ggpo, inputs, ggpo_session->_game_players*sizeof(int), &disconnect_flags);

    py::list disconnected_players;
    py::list player_inputs;

    for (int i=0;i<ggpo_session->_game_players;i++) {
        // Pass disconnected players to the advance frame function
        bool has_disconnected = disconnect_flags<<i&1;
        if (has_disconnected)
            disconnected_players.append(ggpo_session->_py_players[i]);
        player_inputs.append(inputs[i]);
    }

    PyObject_Call(ggpo_session->py_callbacks.advance_frame, py::make_tuple(player_inputs, disconnected_players).ptr(), NULL);
    return true;
}

bool __cdecl _cbk_load_game_state(unsigned char *buffer, int len)
{
    ggpo_session->_game_state = ggpo_session->util_ref_to_pygamestate(buffer, len);
    return true;
}

bool __cdecl _cbk_save_game_state(unsigned char **buffer, int *len, int *checksum, int)
{
    py::bytes b_serialized_gamestate = ggpo_session->util_pygamestate_to_str(ggpo_session->_game_state);

    *len = (py::len(b_serialized_gamestate))*sizeof(char);

    *buffer = (unsigned char*)malloc(*len);
    if (!*buffer)
        return false;

    memset(*buffer, 0, *len);
    memcpy(*buffer, ((std::string)b_serialized_gamestate).c_str(), *len);
    *checksum = fletcher32_checksum((short*)*buffer, *len/2);
    return true;
}

bool __cdecl _cbk_log_game_state(char *filename, unsigned char *buffer, int len)
{
    if (ggpo_session->py_callbacks.log_game_state_def)
        PyObject_Call(ggpo_session->py_callbacks.advance_frame, py::make_tuple(ggpo_session->util_ref_to_pygamestate(buffer, len)).ptr(), NULL);
    return true;
}

void __cdecl _cbk_free_buffer(void *buffer)
{
    free(buffer);
}


bool __cdecl _cbk_on_event(GGPOEvent *e)
{
    if (!ggpo_session->py_callbacks.on_event_def)
        return true;

    py::object py_event = py::module_::import("ggpo_py").attr("GGPOEvent")();
    py_event.attr("__setattr__")("code", py::cast((int)e->code));

    switch (e->code) {
        case GGPO_EVENTCODE_CONNECTED_TO_PEER:
            py_event.attr("__setattr__")("player", (py::object)ggpo_session->util_player_by_ggpoplayerhandle(e->u.connected.player));
            break;
        case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
            py_event.attr("__setattr__")("player", (py::object)ggpo_session->util_player_by_ggpoplayerhandle(e->u.connection_interrupted.player));
            py_event.attr("__setattr__")("disconnect_timeout", py::cast((int)e->u.connection_interrupted.disconnect_timeout));
            break;
        case GGPO_EVENTCODE_CONNECTION_RESUMED:
            py_event.attr("__setattr__")("player", (py::object)ggpo_session->util_player_by_ggpoplayerhandle(e->u.connection_resumed.player));
            break;
        case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
            py_event.attr("__setattr__")("player", (py::object)ggpo_session->util_player_by_ggpoplayerhandle(e->u.disconnected.player));
            break;
        case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
            py_event.attr("__setattr__")("player", (py::object)ggpo_session->util_player_by_ggpoplayerhandle(e->u.synchronized.player));
            break;
        case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
            py_event.attr("__setattr__")("player", (py::object)ggpo_session->util_player_by_ggpoplayerhandle(e->u.synchronizing.player));
            py_event.attr("__setattr__")("count", py::cast((int)e->u.synchronizing.count));
            py_event.attr("__setattr__")("total", py::cast((int)e->u.synchronizing.total));
            break;
        case GGPO_EVENTCODE_TIMESYNC:
            break;
        default:
            break;
    }

    PyObject_Call(ggpo_session->py_callbacks.on_event, py::make_tuple(py_event).ptr(), NULL);
    return true;
}

void PyGGPOSession::py_cbk_advance_frame(py::function &func)
{
    py_callbacks.advance_frame_def = true;
    py_callbacks.advance_frame = func.ptr();
    Py_INCREF(func.ptr());
}

void PyGGPOSession::py_cbk_log_game_state(py::function &func)
{
    py_callbacks.log_game_state_def = true;
    py_callbacks.log_game_state = func.ptr();
    Py_INCREF(func.ptr());
}

void PyGGPOSession::py_cbk_on_event(py::function &func)
{
    py_callbacks.on_event_def = true;
    py_callbacks.on_event = func.ptr();
    Py_INCREF(func.ptr());
}

PyGGPOSession::~PyGGPOSession()
{
    if (_session_running) {
        stop();
        if (py_callbacks.on_event_def)
            Py_DECREF(py_callbacks.on_event);
        if (py_callbacks.advance_frame_def)
            Py_DECREF(py_callbacks.advance_frame);
        if (py_callbacks.log_game_state_def)
            Py_DECREF(py_callbacks.log_game_state);
    }
}

py::object PyGGPOSession::start(void)
{
    if (_session_running)
        throw std::runtime_error("Session is already running.");
    if (!py_callbacks.advance_frame_def)
        throw std::runtime_error("advance_frame callback not defined.");

     int errorcode = ggpo_start_session(
            &this->_ggpo,
            &this->_ggpo_callbacks,
            this->_game_title,
            this->_game_players,
            sizeof(int),
            this->_game_port);

     if (errorcode == GGPO_ERRORCODE_SUCCESS)
         this->_session_running = true;
    
    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);
}

py::object PyGGPOSession::stop(void)
{
    if (!this->_session_running)
        throw std::runtime_error("Session isn't running.");

    int errorcode = ggpo_close_session(this->_ggpo);
    if (errorcode == GGPO_ERRORCODE_SUCCESS)
        this->_session_running = false;

    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);
}

py::object PyGGPOSession::util_player_by_ggpoplayerhandle(GGPOPlayerHandle handle) {
    for (int i=0;i<_game_players;i++) {
        if (_ggpo_playerhandles[i] == handle)
            return _py_players[i];
    }

    throw std::runtime_error("Invalid player looked up.");
}

py::object PyGGPOSession::util_ref_to_pygamestate(void *buffer, int sz) {
    py::module pickle = py::module_::import("pickle");
    py::object game_state = pickle.attr("loads")(py::bytes((char*)buffer, sz));

    return game_state;
}

py::bytes PyGGPOSession::util_pygamestate_to_str(py::object py_gamestate) {
    // serializing the data with pickle
    py::module pickle = py::module_::import("pickle");
    py::bytes serialized = pickle.attr("dumps")(py_gamestate);
    return serialized;
}

PyGGPOErrorCode PyGGPOSession::set_disconnect_timeout(int time_ms) {
    int errorcode;
    errorcode = ggpo_set_disconnect_timeout(_ggpo, time_ms);
    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);
}

PyGGPOErrorCode PyGGPOSession::set_disconnect_notify_start(int time_ms) {
    int errorcode;
    errorcode = ggpo_set_disconnect_notify_start(_ggpo, time_ms);
    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);
}

PyGGPOErrorCode PyGGPOSession::set_frame_delay(py::object player, int frames) {
    int errorcode;
    errorcode = ggpo_set_frame_delay(_ggpo, util_ggpoplayerhandle_by_player(player), frames);
    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);
}

PyGGPOErrorCode PyGGPOSession::disconnect_player(py::object player) {
    int errorcode;
    errorcode = ggpo_disconnect_player(_ggpo, util_ggpoplayerhandle_by_player(player));
    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);
}

PyGGPOErrorCode PyGGPOSession::advance_frame() {
    int errorcode;
    errorcode = ggpo_advance_frame(_ggpo);
    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);
}

PyGGPOErrorCode PyGGPOSession::add_local_input(py::object player, int input) {
    int errorcode;
    errorcode = ggpo_add_local_input(_ggpo, util_ggpoplayerhandle_by_player(player), &input, sizeof(int));
    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);
}

PyGGPOErrorCode PyGGPOSession::idle(int time_ms) {
    int errorcode;
    errorcode = ggpo_idle(_ggpo, time_ms);
    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);
}

GGPOPlayerHandle PyGGPOSession::util_ggpoplayerhandle_by_player(py::object player) {
    for (int i=0;i<_game_players;i++) {
        if (py::cast<int>(_py_players[i].attr("player_nr")) == py::cast<int>(player.attr("player_nr")))
            return _ggpo_playerhandles[i];
    }

    throw std::runtime_error("Player is not in list of players.");
}


py::tuple PyGGPOSession::synchronize_input() {
    int errorcode;
    int *inputs = new int[_game_players];
    int disconnect_flags;
    py::tuple player_inputs = py::tuple(_game_players);

    errorcode = ggpo_synchronize_input(_ggpo, inputs, _game_players*sizeof(int), &disconnect_flags);

    int n_disconnected = 0;
    // disconnected players count
    for (int i=0;i<_game_players;i++) {
        if (disconnect_flags<<i&1)
            n_disconnected++;
    }

    py::tuple disconnected_players = py::tuple(n_disconnected);

    for (int i = 0,j=0;i<_game_players;i++) {
        player_inputs[i] = inputs[i];

        // check if player with that ID has disconnected
        if (disconnect_flags<<i&1) {
            disconnected_players[j] = _py_players[i];
            ++j;
        }
    }

    py::object err = py::module_::import("ggpo_py").attr("GGPOErrorCode")(errorcode);;
    return py::make_tuple(err, py::make_tuple(player_inputs, disconnected_players));
}