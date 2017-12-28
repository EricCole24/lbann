////////////////////////////////////////////////////////////////////////////////xecu
// Copyright (c) 2014-2016, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory.
// Written by the LBANN Research Team (B. Van Essen, et al.) listed in
// the CONTRIBUTORS file. <lbann-dev@llnl.gov>
//
// LLNL-CODE-697807.
// All rights reserved.
//
// This file is part of LBANN: Livermore Big Artificial Neural Network
// Toolkit. For details, see http://software.llnl.gov/LBANN or
// https://github.com/LLNL/LBANN.
//
// Licensed under the Apache License, Version 2.0 (the "Licensee"); you
// may not use this file except in compliance with the License.  You may
// obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the license.
//
// lbann_base .hpp - Basic definitions, functions
////////////////////////////////////////////////////////////////////////////////

#ifndef LBANN_BASE_HPP
#define LBANN_BASE_HPP

#include "El.hpp"
#include "lbann/Elemental_extensions.hpp"

// Datatype for numerical computation
// Default: float
#if LBANN_DATATYPE == 8
using DataType = double;
#elif LBANN_DATATYPE == 4
using DataType = float;
#else
#define LBANN_DATATYPE 4
using DataType = float; 
#endif

// Datatype for model evaluation
// Examples: timing, metrics, objective functions
using EvalType = double;

// Elemental matrices
using EGrid = El::Grid;
using Grid = El::Grid;
using Mat = El::Matrix<DataType>;
using AbsDistMat = El::AbstractDistMatrix<DataType>;
using DistMat = El::DistMatrix<DataType, El::MC, El::MR>;
using CircMat = El::DistMatrix<DataType, El::CIRC, El::CIRC>;
using StarMat = El::DistMatrix<DataType, El::STAR, El::STAR>;
using ColSumMat = El::DistMatrix<DataType, El::MR, El::STAR>; /* Summary matrix over columns */
using RowSumMat = El::DistMatrix<DataType, El::MC, El::STAR>;
using StarVCMat = El::DistMatrix<DataType, El::STAR, El::VC>;
using StarMRMat = El::DistMatrix<DataType, El::STAR, El::MR>;
using ColSumStarVCMat = El::DistMatrix<DataType, El::VC, El::STAR>; /* Summary matrix over columns */
using BlockMat = El::BlockMatrix<DataType>;
using ElMat = El::ElementalMatrix<DataType>;

/// Distributed matrix format
enum class matrix_format {MC_MR, CIRC_CIRC, STAR_STAR, STAR_VC, MC_STAR, invalid};

/// Data layout that is optimized for different modes of parallelism
enum class data_layout {MODEL_PARALLEL, DATA_PARALLEL, invalid};
static matrix_format __attribute__((used)) data_layout_to_matrix_format(data_layout layout) {
  matrix_format format;
  switch(layout) {
  case data_layout::MODEL_PARALLEL:
    format = matrix_format::MC_MR;
    break;
  case data_layout::DATA_PARALLEL:
    /// Weights are stored in STAR_STAR and data in STAR_VC
    format = matrix_format::STAR_STAR;
    break;
  default:
    throw(std::string{} + __FILE__ + " " + std::to_string(__LINE__) + " Invalid data layout selected");
  }
  return format;
}

/// Neural network execution mode
enum class execution_mode {training, validation, testing, prediction, invalid};
static const char *__attribute__((used)) _to_string(execution_mode m) {
  switch(m) {
  case execution_mode::training:
    return "training";
  case execution_mode::validation:
    return "validation";
  case execution_mode::testing:
    return "testing";
  case execution_mode::prediction:
    return "prediction";
  case execution_mode::invalid:
    return "invalid";
  default:
    throw("Invalid execution mode specified"); /// @todo this should be an lbann_exception but then the class has to move to resolve dependencies
  }
}

/** Pooling layer mode */
enum class pool_mode {max, average, average_no_pad};

/** returns a string representation of the pool_mode */
std::string get_pool_mode_name(pool_mode m); 

namespace lbann {

// Forward-declaration.
class lbann_comm;

/**
 * Initialize LBANN.
 * The comm instance this returns places every process in one model. This can be
 * changed with lbann_comm::split_models afterward.
 * @param argc The program's argc.
 * @param argv The program's argv.
 * @param seed Optional seed for random number generators.
 */
lbann_comm* initialize(int& argc, char**& argv, int seed = -1);
/**
 * Perform finalization.
 */
void finalize(lbann_comm* comm = nullptr);

class CUtility {
 public:
  static void convolveMat(StarMat *Kernels, BlockMat& InputMat, BlockMat& OutputMat,
                          uint InputWidth, uint InputHeight);
};

}  // namespace lbann

/// Print the dimensions and name of a Elemental matrix
static void __attribute__((used)) _display_matrix(ElMat *m, const char *name) {
  std::cout << "DISPLAY MATRIX: " << name << " = " << m->Height() << " x " << m->Width() << std::endl;
}
#define DISPLAY_MATRIX(x) _display_matrix(x, #x);

#ifndef DEBUG
#define DEBUG 1 // set debug mode
#endif

#if DEBUG
// __FILE__
#define log_msg(...) {\
  char str[256];\
  sprintf(str, __VA_ARGS__);\
  std::cout << "[" << comm->get_model_rank() << "." << comm->get_rank_in_model() << "][" << __FUNCTION__ << "][Line " << __LINE__ << "]" << str << std::endl; \
  }
#define log_simple_msg(...) {\
  char str[256];\
  sprintf(str, __VA_ARGS__);\
  std::cout << "[" << __FUNCTION__ << "][Line " << __LINE__ << "]" << str << std::endl; \
  }
#else
#define log_msg(...)
#define log_simple_msg(...)
#endif

#define LBANN_MAKE_STR(x) _LBANN_MAKE_STR(x)
#define _LBANN_MAKE_STR(x) #x

#endif // LBANN_BASE_HPP
