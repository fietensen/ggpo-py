#ifndef GGPO_PY_PY_GGPOSESSION_H
#define GGPO_PY_PY_GGPOSESSION_H
#include <vector>
#include <pybind11/pybind11.h>
#include "ggponet.h"

namespace py = pybind11;

typedef py::object PyGGPOErrorCode;

// callbacks
bool __cdecl _cbk_begin_game(const char*);
bool __cdecl _cbk_advance_frame(int);
bool __cdecl _cbk_load_game_state(unsigned char*, int);
bool __cdecl _cbk_save_game_state(unsigned char**, int*, int*, int);
bool __cdecl _cbk_log_game_state(char*,unsigned char*,int);
void __cdecl _cbk_free_buffer(void*);
bool __cdecl _cbk_on_event(GGPOEvent*);


typedef struct PyGGPOCallbacks {
    // optional callbacks
    PyObject *on_event;
    PyObject *log_game_state;

    bool on_event_def = false;
    bool log_game_state_def = false;

    // required callbacks
    PyObject *advance_frame;
    bool advance_frame_def = false;

    PyObject *on_rollback;
    bool on_rollback_def = false;
} PyGGPOCallbacks;

class PyGGPOSession {
public:
    PyGGPOSession(const char*, unsigned short, int, py::object);
    ~PyGGPOSession();

    py::object start(void);
    py::object stop(void);

    GGPOSession *_ggpo;
    GGPOSessionCallbacks _ggpo_callbacks;

    py::object *_py_players;
    GGPOPlayer *_ggpo_players;
    GGPOPlayerHandle *_ggpo_playerhandles;

    py::object _game_state;

    PyGGPOCallbacks py_callbacks;

    const char *_game_title;
    int _game_players;
    unsigned short _game_port;
    bool _session_running = false;


    py::object util_player_by_ggpoplayerhandle(GGPOPlayerHandle);
    py::object util_ref_to_pygamestate(void*,int);

    GGPOPlayerHandle util_ggpoplayerhandle_by_player(py::object);
    py::bytes util_pygamestate_to_str(py::object);

    // callback setter functions
    void py_cbk_advance_frame(py::function&);
    void py_cbk_log_game_state(py::function&);
    void py_cbk_on_event(py::function&);
    void py_cbk_on_rollback(py::function&);

    // ggpo function wrapper
    py::object add_player(py::object);
    PyGGPOErrorCode set_disconnect_timeout(int);
    PyGGPOErrorCode set_disconnect_notify_start(int);
    PyGGPOErrorCode set_frame_delay(py::object, int);
    PyGGPOErrorCode disconnect_player(py::object);
    PyGGPOErrorCode advance_frame();
    PyGGPOErrorCode add_local_input(py::object, int);
    PyGGPOErrorCode idle(int);
    py::tuple synchronize_input();

};

#endif //GGPO_PY_PY_GGPOSESSION_H
