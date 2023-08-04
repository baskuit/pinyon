#include <pybind11/pybind11.h>

#include <surskit.hh>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(pysurskit, m) {

    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: cmake_example

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

    py::class_<prng>(m, "prng")
    .def(py::init<>())
    .def(py::init<int>())
    .def("uniform", &prng::uniform);

    py::class_<RandomTree>(m, "RandomTree").def(py::init<prng, int, int, int, int, double>());

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
