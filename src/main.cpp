#include <pybind11/pybind11.h>
#include <iostream>
#include "py_ggposession.h"

namespace py = pybind11;

PYBIND11_MODULE(ggpo_py, m) {
    m.doc() = "Python bindings to GGPO";

    py::class_<PyGGPOSession>(m, "GGPOSession")
            .def(py::init<const char*, unsigned short, int, py::object>(),
                    py::arg("game_title"),
                    py::arg("port"),
                    py::arg("n_players"),
                    py::arg("game_status"))
            .def("start", &PyGGPOSession::start)
            .def("stop", &PyGGPOSession::stop)
            .def("add_player", &PyGGPOSession::add_player)
            .def("cbk_advance_frame", &PyGGPOSession::py_cbk_advance_frame)
            .def("cbk_log_game_state", &PyGGPOSession::py_cbk_log_game_state)
            .def("cbk_on_event", &PyGGPOSession::py_cbk_on_event)
            .def("add_player", &PyGGPOSession::add_player)
            .def("set_disconnect_timeout", &PyGGPOSession::set_disconnect_timeout)
            .def("set_disconnect_notify_start", &PyGGPOSession::set_disconnect_notify_start)
            .def("set_frame_delay", &PyGGPOSession::set_frame_delay)
            .def("disconnect_player", &PyGGPOSession::disconnect_player)
            .def("advance_frame", &PyGGPOSession::advance_frame)
            .def("add_local_input", &PyGGPOSession::add_local_input)
            .def("idle", &PyGGPOSession::idle)
            .def("synchronize_input", &PyGGPOSession::synchronize_input);
}