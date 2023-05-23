#ifndef GGPO_PY_PY_GGPOSESSION_H
#define GGPO_PY_PY_GGPOSESSION_H
#include <vector>
#include <pybind11/pybind11.h>
#include "ggponet.h"

namespace py = pybind11;

typedef struct PyGGPOCallbacks {
    py::function &on_event;
    py::function &advance_frame;
    py::function &free_buffer;
    py::function &load_game_state;
    py::function &save_game_state;
    py::function &log_game_state;

    bool on_event_def = false;
    bool advance_frame_def = false;
    bool free_buffer_def = false;
    bool load_game_state_def = false;
    bool save_game_state_def = false;
    bool log_game_state_def = false;
} PyGGPOCallbacks;

class PyGGPOSession {
    GGPOSession *_ggpo;
    GGPOSessionCallbacks *_ggpo_callbacks;

    PyGGPOCallbacks *py_callbacks;

    const char *_game_title;
    int _game_players;
    unsigned short _game_port;
    bool _session_running = false;

    bool __cdecl _cbk_begin_game(const char*);
    void _cbk_advance_frame(void);
    void _cbk_load_game_state(void);
    void _cbk_save_game_state(void);
    void _cbk_log_game_state(void);
    void _cbk_free_buffer(void);
    void _cbk_on_event(void);

public:
    PyGGPOSession(const char*, int, unsigned short);
    ~PyGGPOSession();

    py::object start(void);
    py::object stop(void);

    // callback setter functions
    void py_cbk_begin_game(py::function &func);
    void py_cbk_advance_frame(py::function &func);
    void py_cbk_load_game_state(py::function &func);
    void py_cbk_save_game_state(py::function &func);
    void py_cbk_log_game_state(py::function &func);
    void py_cbk_free_buffer(py::function&);
    void py_cbk_on_event(py::function&);
};

#endif //GGPO_PY_PY_GGPOSESSION_H
