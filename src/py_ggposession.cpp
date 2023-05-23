#include "py_ggposession.h"
#include <iostream>


PyGGPOSession::PyGGPOSession(const char *game_title, int n_players, unsigned short game_port)
{
    _game_title = game_title;
    _game_players = n_players;
    _game_port = game_port;

    /*
    _ggpo_callbacks->save_game_state = _cbk_save_game_state;
    _ggpo_callbacks->load_game_state = _cbk_load_game_state;
    _ggpo_callbacks->free_buffer = _cbk_free_buffer;
    _ggpo_callbacks->advance_frame = _cbk_advance_frame;
    _ggpo_callbacks->on_event = _cbk_on_event;
    _ggpo_callbacks->begin_game = _cbk_begin_game;
    _ggpo_callbacks->log_game_state = _cbk_log_game_state;*/
}

bool __cdecl _cbk_begin_game(const char*)
{
    return true;
}

void _cbk_advance_frame(void)
{

}

void _cbk_load_game_state(void)
{

}

void _cbk_save_game_state(void)
{

}

void _cbk_log_game_state(void)
{

}

void _cbk_free_buffer(void)
{

}


void _cbk_on_event(void)
{

}

void PyGGPOSession::py_cbk_advance_frame(py::function &func)
{

}

void PyGGPOSession::py_cbk_load_game_state(py::function &func)
{

}

void PyGGPOSession::py_cbk_save_game_state(py::function &func)
{

}

void PyGGPOSession::py_cbk_free_buffer(py::function &func)
{

}

void PyGGPOSession::py_cbk_on_event(py::function &func)
{

}

PyGGPOSession::~PyGGPOSession(void)
{
    if (_session_running)
        stop();
}

py::object PyGGPOSession::start(void)
{
    if (this->_session_running)
        throw std::runtime_error("Session is already running.");




     int errorcode = ggpo_start_session(
            &this->_ggpo,
            this->_ggpo_callbacks,
            this->_game_title,
            this->_game_players,
            sizeof(int),
            this->_game_port);
    
    return py::module_::import("gppo_py").attr("GGPOErrorCode")(errorcode);
}

py::object PyGGPOSession::stop(void)
{
    if (!this->_session_running)
        throw std::runtime_error("Session isn't running.");

    int errorcode = ggpo_close_session(this->_ggpo);

    return py::module_::import("gppo_py").attr("GGPOErrorCode")(errorcode);
}