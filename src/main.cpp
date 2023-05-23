#include <pybind11/pybind11.h>
#include "py_ggposession.h"

namespace py = pybind11;

py::object test(int a) {
    return py::module_::import("ggpo_py").attr("GGPOErrorCode")(a);
}

PYBIND11_MODULE(ggpo_py, m) {
    m.doc() = "Python bindings to GGPO";
    m.def("test", &test);
    py::class_<PyGGPOSession>(m, "GGPOSession")
            .def(py::init<const char*, int, int>(),
                    py::arg("game_title"),
                    py::arg("n_players"),
                    py::arg("port"));
}