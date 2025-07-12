/*
    Parallel Systems Exercise Batch 2 -- Solutions to Batch 2 of Exercises for
   the Parallel Systems Course of the "Computer Engineering" Masters Programme
   of NKUA Copyright (C) 2025 Christoforos-Marios Mamaloukas

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "exercises.hxx"

#include <chrono>
#include <iostream>

#include "mpi.h"

#include "game_of_life_common.hxx"

using Dim = utility::Matrix<short>::Dim;

// --- FUNCTION DEFINITIONS --- //

auto exe::game_of_life_mix(const utility::Options &options)
    -> utility::ExerciseVariantReturn {
  int32_t rank{0};
  int32_t nodes{0};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nodes);

  const auto specifics{
      std::get<utility::ExerciseOptions<utility::Exercise::GAME_OF_LIFE_MIX>>(
          options.specifics)};
  auto matrix = utility::Matrix<short>(specifics.dims[0], specifics.dims[1]);

  const auto start{std::chrono::time_point_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now())};
  if (rank == 0) {
    // we generate the matrix in process 0 to broadcast it afterwards to the
    // other processes
#pragma omp parallel num_threads(specifics.jobs)
    for (uint32_t i{0}; i < matrix.size<Dim::Y>(); i++) {
#pragma omp for
      for (uint32_t j = 0; j < matrix.size<Dim::X>(); j++) {
        const auto time{std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now())};
        const auto duration{time.time_since_epoch().count()};
        std::srand(duration);

        auto value{static_cast<double>(std::rand()) /
                   static_cast<double>(RAND_MAX)};

        matrix(j, i) = static_cast<short>(value <= 0.5);
      }
    }
  }

  // This does NOT represent how many elements each node will handle, as
  // `nodes` may not divide the matrix size, however it's a helpful
  // intermediate variable
  const auto &&per_node_elem_count{matrix.size<Dim::X>() *
                                   matrix.size<Dim::Y>() / nodes};

  std::vector<uint32_t> top(nodes), left(nodes), bottom(nodes), right(nodes),
      upper_col(nodes), lower_col(nodes), chunk_size(nodes), elem_count(nodes);

  for (uint32_t i{0}; i < nodes; i++) {
    top[i] = i == 0 ? 0
                    : (i - 1) * per_node_elem_count /
                          matrix.size<Dim::X>(); // The Y coordinate of the
                                                 // first element of the node
    left[i] = i == 0 ? 0
                     : (i - 1) * per_node_elem_count %
                           matrix.size<Dim::X>(); // The X coordinate of the
                                                  // first element of the node
    bottom[i] = i == nodes - 1
                    ? matrix.size<Dim::Y>() - 1
                    : i * per_node_elem_count /
                          matrix.size<Dim::X>(); // The Y coordinate of the
                                                 // last element of the node
    right[i] = i == nodes - 1
                   ? matrix.size<Dim::X>() - 1
                   : i * per_node_elem_count %
                         matrix.size<Dim::X>(); // The X coordinate of the
                                                // last element of the node

    elem_count[i] = (1 + bottom[i] - top[i]) * matrix.size<Dim::X>() - left[i] -
                    (matrix.size<Dim::X>() - right[i]);

    // these two represent the highest and lowest columns that will be sent to
    // the i'th node
    upper_col[i] = top[i] == 0 ? top[i] : top[i] - 1;
    lower_col[i] =
        bottom[i] == matrix.size<Dim::Y>() - 1 ? bottom[i] : bottom[i] + 1;

    chunk_size[i] = (lower_col[i] - upper_col[i]) * matrix.size<Dim::X>();
  }

#pragma omp parallel num_threads(specifics.jobs)
  for (uint32_t gen{0}; gen < specifics.generations; gen++) {
    if (rank == 0) {
#pragma omp single
      for (uint32_t i{1}; i < static_cast<uint32_t>(nodes); i++) {
        MPI_Send(reinterpret_cast<void *>(&matrix(0, upper_col[i])),
                 chunk_size[i], MPI_SHORT, i, 0, MPI_COMM_WORLD);
      }
    } else {
#pragma omp single
      MPI_Recv(reinterpret_cast<void *>(&matrix(0, upper_col[rank])),
               chunk_size[rank], MPI_SHORT, 0, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
    }

#pragma omp for schedule(dynamic, 1)
    for (uint32_t i = top[rank]; i < bottom[rank]; i++) {
      for (uint32_t j = left[rank]; j < right[rank]; j++) {
        matrix(j, i) = static_cast<short>(gol::check(matrix, j, i));
      }
    }

    // we only receive from every node the elements it modifies
    if (rank == 0) {
#pragma omp single
      for (uint32_t i{1}; i < static_cast<uint32_t>(nodes); i++) {
        MPI_Recv(reinterpret_cast<void *>(&matrix(top[i], left[i])),
                 elem_count[i], MPI_SHORT, i, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
      }
    } else {
#pragma omp single
      MPI_Send(reinterpret_cast<void *>(&matrix(top[rank], left[rank])),
               elem_count[rank], MPI_SHORT, 0, 0, MPI_COMM_WORLD);
    }
#pragma omp barrier
  }

  const auto end{std::chrono::time_point_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now())};

  return utility::ExerciseReturn<utility::Exercise::GAME_OF_LIFE_MIX>{
      .time{static_cast<uint64_t>((end - start).count())},
      .dims{specifics.dims[0], specifics.dims[1]},
  };
}
