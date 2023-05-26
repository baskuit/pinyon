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

    py::class_<EmptyClass>(m, "EmptyClass").def(py::init<>());

    py::class_<prng>(m, "prng").def(py::init<>()).def(py::init<int>());

    py::class_<RandomTree>(m, "RandomTree").def(py::init<prng, int, int, int, int, double>())
        .def("__dir__", [](const RandomTree& obj) {
            py::list result;
            py::handle scope = py::getattr(py::module::import("__main__"), "__dict__");
            auto class_dict = py::getattr(scope, "RandomTree");
            py::dict dict(class_dict);
            for (auto item : dict) {
                if (py::isinstance<py::function>(item.second)) {
                    result.append(item.first);
                }
            }
            return result;
        });
    

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
