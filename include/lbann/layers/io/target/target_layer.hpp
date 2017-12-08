////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////

#ifndef LBANN_LAYERS_TARGET_LAYER_HPP_INCLUDED
#define LBANN_LAYERS_TARGET_LAYER_HPP_INCLUDED

#include "lbann/layers/io/io_layer.hpp"
#include "lbann/layers/io/input/input_layer.hpp"
#include "lbann/utils/exception.hpp"
#include "lbann/models/model.hpp"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace lbann {
class target_layer : public io_layer {
 protected:
  input_layer *paired_input_layer;

 public:
  target_layer(lbann_comm *comm, input_layer* input_layer, std::map<execution_mode, generic_data_reader *> data_readers, bool for_regression = false)
    : io_layer(comm, true, for_regression), paired_input_layer(input_layer) {
    // Target layers have no children
    m_max_num_child_layers = 0;
  }

  ~target_layer() override = default;

  target_layer(const target_layer& other) = default;

  target_layer& operator=(const target_layer& other) = default;

  template<data_layout T_layout> inline void initialize_distributed_matrices() {
    io_layer::initialize_distributed_matrices<T_layout>();
  }

  input_layer* get_paired_input_layer() {
    return paired_input_layer;
  }

  void set_paired_input_layer(input_layer *input_layer) {
    paired_input_layer = input_layer;
  }

  void setup_dims() override {
    io_layer::setup_dims();
    if (this->is_for_regression()) {
      this->m_neuron_dims = get_data_dims();
      this->m_num_neuron_dims = this->m_neuron_dims.size();
      this->m_num_neurons = std::accumulate(this->m_neuron_dims.begin(),
                                            this->m_neuron_dims.end(),
                                            1,
                                            std::multiplies<int>());
    } else {
      this->m_num_neurons = get_linearized_label_size();
      this->m_num_neuron_dims = 1;
      this->m_neuron_dims.assign(1, this->m_num_neurons);
    }
  }

  void setup_data() override {
    io_layer::setup_data();
    std::stringstream err;

    if(this->m_num_prev_neurons != this->m_num_neurons) {
      err << __FILE__ << " " << __LINE__ 
          << " :: " << get_type() << " this->m_num_prev_neurons != this->m_num_neurons; this->m_num_prev_neurons= " << this->m_num_prev_neurons << " this->m_num_neurons= " << this->m_num_neurons << std::endl;
      throw lbann_exception(err.str());
    }

    for (auto&& m : this->m_model->get_metrics()) {
      m->setup(this->m_num_neurons,
               this->m_model->get_max_mini_batch_size());
    }

  }

  // lbann::generic_data_reader *set_training_data_reader(generic_data_reader *data_reader, bool shared_data_reader) {
  //   return io_layer::set_training_data_reader(data_reader);
  // }

  // lbann::generic_data_reader *set_testing_data_reader(generic_data_reader *data_reader, bool shared_data_reader) {
  //   return io_layer::set_testing_data_reader(data_reader);
  // }

  void fp_set_std_matrix_view() override {
    int cur_mini_batch_size = this->m_model->get_current_mini_batch_size();
    Layer::fp_set_std_matrix_view();
    for (auto&& m : this->m_model->get_metrics()) {
      m->fp_set_std_matrix_view(cur_mini_batch_size);
    }
  }
  //************************************************************************
  // Helper functions to access the data readers
  //************************************************************************
  dataset& get_dataset(execution_mode m) override {
    return paired_input_layer->get_dataset(m);
  }

  const dataset& get_dataset(execution_mode m) const override {
    return paired_input_layer->get_dataset(m);
  }

  /**
   * Return the dataset associated with the current execution mode.
   */
  dataset& select_dataset() override { return paired_input_layer->select_dataset(); }
  const dataset& select_dataset() const override { return paired_input_layer->select_dataset(); }

  /**
   * Return the first dataset with a valid (non-null) datareader.
   * Returns null if none are valid.
   */
  dataset* select_first_valid_dataset() override {
    return paired_input_layer->select_first_valid_dataset();
  }

  /**
   * Return the data reader associated with the current execution mode.
   */
  generic_data_reader *select_data_reader() const override {
    return paired_input_layer->select_data_reader();
  }

  /**
   * Update the number of samples processed for the current execution mode.
   */
  long update_num_samples_processed(long num_samples) override {
    return paired_input_layer->update_num_samples_processed(num_samples);
  }

  /**
   * Return the sample indices fetched in the current mini-batch.
   */
  El::Matrix<El::Int>* get_sample_indices_per_mb() override {
    return paired_input_layer->get_sample_indices_per_mb();
  }

  /**
   * Get the dimensions of the underlying data.
   */
  const std::vector<int> get_data_dims() const override {
    return paired_input_layer->get_data_dims();
  }

  std::string get_topo_description() const override {
    return paired_input_layer->get_topo_description();
  }

  /**
   * Get the linearized size of the underlying data.
   */
  long get_linearized_data_size() const override {
    return paired_input_layer->get_linearized_data_size();
  }

  /**
   * Get the linearized size of the labels for the underlying data.
   */
  long get_linearized_label_size() const override {
    return paired_input_layer->get_linearized_label_size();
  }

  long get_linearized_response_size() const override {
    return paired_input_layer->get_linearized_response_size();
  }

  long get_num_samples_trained() const override {
    return paired_input_layer->get_num_samples_trained();
  }
  long get_num_samples_tested() const override {
    return paired_input_layer->get_num_samples_tested();
  }
  long get_total_num_training_samples() const override {
    return paired_input_layer->get_total_num_training_samples();
  }
  long get_total_num_testing_samples() const override {
    return paired_input_layer->get_total_num_testing_samples();
  }

  bool at_new_epoch() const override {
    return paired_input_layer->at_new_epoch();
  }

  bool is_execution_mode_valid(execution_mode mode) const override {
    return paired_input_layer->is_execution_mode_valid(mode);
  }

  AbsDistMat& get_prediction() { return *this->m_prev_activations_v; }
  AbsDistMat& get_ground_truth() { return *this->m_activations_v; }

  std::vector<Layer*> get_layer_pointers() override {
    std::vector<Layer*> layers = io_layer::get_layer_pointers();
    layers.push_back((Layer*) paired_input_layer);
    return layers;
  }

  void set_layer_pointers(std::vector<Layer*> layers) override {
    paired_input_layer = dynamic_cast<input_layer*>(layers.back());
    if (paired_input_layer == nullptr) {
      std::stringstream err;
      err << __FILE__ << " " << __LINE__ 
          << " :: lbann_target_layer: invalid layer pointer used to set paired input layer";
      throw lbann_exception(err.str());
    }
    layers.pop_back();
    io_layer::set_layer_pointers(layers);
  }

  //************************************************************************
  //
  //************************************************************************

  bool saveToCheckpoint(int fd, const char *filename, size_t *bytes) const override {
    /// @todo should probably save m_shared_data_reader
    return Layer::saveToCheckpoint(fd, filename, bytes);
  }

  bool loadFromCheckpoint(int fd, const char *filename, size_t *bytes) override {
    /// @todo should probably save m_shared_data_reader
    return Layer::loadFromCheckpoint(fd, filename, bytes);
  }

  bool saveToCheckpointShared(persist& p) const override {
    // rank 0 writes softmax cost to file
    if (p.get_rank() == 0) {
      // p.write_double(persist_type::train, "aggregate cost", (double) aggregate_cost);
      // p.write_uint64(persist_type::train, "num backprop steps", (uint64_t) num_backprop_steps);
    }

    return true;
  }

  bool loadFromCheckpointShared(persist& p) override {
    // rank 0 writes softmax cost to file
    // if (p.get_rank() == 0) {
    //     double dval;
    //     p.read_double(persist_type::train, "aggregate cost", &dval);
    //     aggregate_cost = (DataType) dval;

    //     uint64_t val;
    //     p.read_uint64(persist_type::train, "num backprop steps", &val);
    //     num_backprop_steps = (long) val;
    // }

    // // get values from rank 0
    // MPI_Bcast(&aggregate_cost, 1, DataTypeMPI, 0, MPI_COMM_WORLD);
    // MPI_Bcast(&num_backprop_steps, 1, MPI_LONG, 0, MPI_COMM_WORLD);

    //return Layer::loadFromCheckpointShared(dir, bytes);
    return true;
  }
};

}  // namespace lbann

#endif  // LBANN_LAYERS_TARGET_LAYER_HPP_INCLUDED
