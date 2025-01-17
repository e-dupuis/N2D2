/*
    (C) Copyright 2013 CEA LIST. All Rights Reserved.
    Contributor(s): Olivier BICHLER (olivier.bichler@cea.fr)

    This software is governed by the CeCILL-C license under French law and
    abiding by the rules of distribution of free software.  You can  use,
    modify and/ or redistribute the software under the terms of the CeCILL-C
    license as circulated by CEA, CNRS and INRIA at the following URL
    "http://www.cecill.info".

    As a counterpart to the access to the source code and  rights to copy,
    modify and redistribute granted by the license, users are provided only
    with a limited warranty  and the software's author,  the holder of the
    economic rights,  and the successive licensors  have only  limited
    liability.

    The fact that you are presently reading this means that you have had
    knowledge of the CeCILL-C license and that you accept its terms.
*/

#ifdef PYBIND
#include "DeepNet.hpp"
#include "HeteroStimuliProvider.hpp"
#include "Cell/Cell.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace N2D2 {
void init_Cell(py::module &m) {
    py::class_<Cell, std::shared_ptr<Cell>> cell(m, "Cell", py::multiple_inheritance());

    py::enum_<Cell::FreeParametersType>(cell, "FreeParametersType")
    .value("Additive", Cell::FreeParametersType::Additive)
    .value("Multiplicative", Cell::FreeParametersType::Multiplicative)
    .value("All", Cell::FreeParametersType::All)
    .export_values();

    cell.def("getId", &Cell::getId)
    .def("getName", &Cell::getName)
    .def("getType", &Cell::getType)
    .def("getNbChannels", &Cell::getNbChannels)
    .def("getChannelsWidth", &Cell::getChannelsWidth)
    .def("getChannelsHeight", &Cell::getChannelsHeight)
    .def("getInputsDim", &Cell::getInputsDim, py::arg("dim"))
    .def("getInputsDims", &Cell::getInputsDims)
    .def("getInputsSize", &Cell::getInputsSize)
    .def("getNbOutputs", &Cell::getNbOutputs)
    .def("getOutputsWidth", &Cell::getOutputsWidth)
    .def("getOutputsHeight", &Cell::getOutputsHeight)
    .def("getOutputsDim", &Cell::getOutputsDim, py::arg("dim"))
    .def("getOutputsDims", &Cell::getOutputsDims)
    .def("getOutputsSize", &Cell::getOutputsSize)
    .def("getStats", &Cell::getStats)
    .def("getReceptiveField", &Cell::getReceptiveField, py::arg("outputField") = std::vector<unsigned int>())
    .def("getAssociatedDeepNet", &Cell::getAssociatedDeepNet)
    .def("isConnection", &Cell::isConnection, py::arg("channel"), py::arg("output"))
    .def("addInput", (void (Cell::*)(StimuliProvider&, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, const Tensor<bool>&)) &Cell::addInput, py::arg("sp"), py::arg("channel"), py::arg("x0"), py::arg("y0"), py::arg("width"), py::arg("height"), py::arg("mapping") = Tensor<bool>())
    .def("addInput", (void (Cell::*)(StimuliProvider&, unsigned int, unsigned int, unsigned int, unsigned int, const Tensor<bool>&)) &Cell::addInput, py::arg("sp"), py::arg("x0") = 0, py::arg("y0") = 0, py::arg("width") = 0, py::arg("height") = 0, py::arg("mapping") = Tensor<bool>())
    .def("addMultiscaleInput", &Cell::addMultiscaleInput, py::arg("sp"), py::arg("x0") = 0, py::arg("y0") = 0, py::arg("width") = 0, py::arg("height") = 0, py::arg("mapping") = Tensor<bool>())
    .def("addInput", (void (Cell::*)(Cell*, const Tensor<bool>&)) &Cell::addInput, py::arg("cell"), py::arg("mapping") = Tensor<bool>())
    .def("addInput", (void (Cell::*)(Cell*, unsigned int, unsigned int, unsigned int, unsigned int)) &Cell::addInput, py::arg("cell"), py::arg("x0"), py::arg("y0"), py::arg("width") = 0, py::arg("height") = 0)
    .def("initialize", &Cell::initialize)
    .def("save", &Cell::save, py::arg("dirName"))
    .def("load", &Cell::load, py::arg("dirName"))
    .def("saveFreeParameters", &Cell::saveFreeParameters, py::arg("fileName"))
    .def("loadFreeParameters", &Cell::loadFreeParameters, py::arg("fileName"), py::arg("ignoreNotExists") = false)
    .def("exportFreeParameters", &Cell::exportFreeParameters, py::arg("fileName"))
    .def("importFreeParameters", &Cell::importFreeParameters, py::arg("fileName"), py::arg("ignoreNotExists") = false)
    .def("logFreeParameters", &Cell::logFreeParameters, py::arg("fileName"))
    .def("logFreeParametersDistrib", &Cell::logFreeParametersDistrib, py::arg("fileName"), py::arg("type"))
    .def("getFreeParametersRange", &Cell::getFreeParametersRange, py::arg("type"))
    .def("getFreeParametersRangePerOutput", &Cell::getFreeParametersRangePerOutput, py::arg("output"), py::arg("type"))
    .def("processFreeParameters", &Cell::processFreeParameters, py::arg("func"), py::arg("type"))
    .def("processFreeParametersPerOutput", &Cell::processFreeParametersPerOutput, py::arg("func"), py::arg("output"), py::arg("type"))
    .def("isFullMap", &Cell::isFullMap)
    .def("groupMap", &Cell::groupMap)
    .def("isUnitMap", &Cell::isUnitMap);
}
}
#endif
