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

auto exe::game_of_life_async(const utility::Options &options)
    -> utility::ExerciseVariantReturn {
  int32_t rank{0};
  int32_t nodes{0};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nodes);

  const auto specifics{
      std::get<utility::ExerciseOptions<utility::Exercise::GAME_OF_LIFE_ASYNC>>(
          options.specifics)};
  auto matrix = utility::Matrix<short>(specifics.dims[0], specifics.dims[1]);

  const auto start{std::chrono::time_point_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now())};
  if (rank == 0) {
    std::cout << R"(--- GAME OF LIFE ---
-> Number of nodes: )"
              << nodes << R"(
-> Number of generations: )"
              << specifics.generations << R"(
-> Matrix size is: )"
              << specifics.dims[0] << R"(x)" << specifics.dims[1] << R"(
-> Data is saved in: )"
              << options.filepath << R"(
-------------------
)";

    // we generate the matrix in process 0 to broadcast it afterwards to the
    // other processes
    for (uint32_t i{0}; i < matrix.size<Dim::Y>(); i++) {
      for (uint32_t j{0}; j < matrix.size<Dim::X>(); j++) {
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

  std::vector<MPI_Request> send_req(nodes), recv_req(nodes);
  for (uint32_t gen{0}; gen < specifics.generations; gen++) {
    // we want the memory each process will get to be continguous
    // This is node independent and can be useful so node 0 can figure out
    // how big of a chunk to send to each other node;
    const auto &&per_node_elem_count{matrix.size<Dim::X>() *
                                     matrix.size<Dim::Y>() / nodes};
    const auto &&my_top{rank == 0 ? 0
                                  : (rank - 1) * per_node_elem_count /
                                        matrix.size<Dim::X>()};
    const auto &&my_left{rank == 0 ? 0
                                   : (rank - 1) * per_node_elem_count %
                                         matrix.size<Dim::X>()};
    const auto &&my_bottom{rank * per_node_elem_count / matrix.size<Dim::X>()};

    // these two represent the highest and lowest columns that will be sent to
    // the i'th node
    const auto &&my_upper_col{my_top == 0 ? 0 : my_top - 1};
    const auto &&my_lower_col{
        my_bottom == matrix.size<Dim::Y>() - 1 ? my_bottom : my_bottom + 1};

    const auto &&recv_elem_count{(my_lower_col - my_upper_col) *
                                 matrix.size<Dim::X>()};

    if (rank == 0) {
      for (uint32_t i{1}; i < static_cast<uint32_t>(nodes); i++) {
        const auto &&its_top{(i - 1) * per_node_elem_count /
                             matrix.size<Dim::X>()};
        const auto &&its_bottom{i * per_node_elem_count /
                                matrix.size<Dim::X>()};
        // these two represent the highest and lowest columns that will be sent
        // to the i'th node
        const auto &&its_upper_col{its_top == 0 ? 0 : its_top - 1};
        const auto &&its_lower_col{its_bottom == matrix.size<Dim::Y>() - 1
                                       ? its_bottom
                                       : its_bottom + 1};

        const auto &&sent_elem_count{(its_lower_col - its_upper_col) *
                                     matrix.size<Dim::X>()};
        MPI_Isend(reinterpret_cast<void *>(&matrix(0, its_upper_col)),
                  sent_elem_count, MPI_SHORT, i, 0, MPI_COMM_WORLD,
                  &send_req[i]);
      }
    } else {
      if (gen != 0) // first generation WON'T have valid send requests
        MPI_Wait(&send_req[0], MPI_STATUS_IGNORE);
      MPI_Irecv(reinterpret_cast<void *>(&matrix(0, my_upper_col)),
                recv_elem_count, MPI_SHORT, 0, 0, MPI_COMM_WORLD, &recv_req[0]);
    }

    if (rank != 0)
      MPI_Wait(&recv_req[0], MPI_STATUS_IGNORE);
    for (uint32_t i{0}; i < per_node_elem_count; i++) {
      auto x{(my_left + i) % matrix.size<Dim::X>()},
          y{(my_left + i) / matrix.size<Dim::Y>()};
      matrix(x, y) = static_cast<short>(gol::check(matrix, x, y));
    }

    if (rank == 0) {
      for (uint32_t i{1}; i < static_cast<uint32_t>(nodes); i++) {
        const auto &&its_top{(i - 1) * per_node_elem_count /
                             matrix.size<Dim::X>()};
        const auto &&its_bottom{i * per_node_elem_count /
                                matrix.size<Dim::X>()};
        // these two represent the highest and lowest columns that will be sent
        // to the i'th node
        const auto &&its_upper_col{its_top == 0 ? 0 : its_top - 1};
        const auto &&its_lower_col{its_bottom == matrix.size<Dim::Y>() - 1
                                       ? its_bottom
                                       : its_bottom + 1};

        const auto &&sent_elem_count{(its_lower_col - its_upper_col) *
                                     matrix.size<Dim::X>()};
        MPI_Wait(&send_req[i], MPI_STATUS_IGNORE);
        MPI_Irecv(reinterpret_cast<void *>(&matrix(0, its_upper_col)),
                  sent_elem_count, MPI_SHORT, i, 0, MPI_COMM_WORLD,
                  &recv_req[i]);
      }
    } else {
      MPI_Isend(reinterpret_cast<void *>(&matrix(0, my_upper_col)),
                recv_elem_count, MPI_SHORT, 0, 0, MPI_COMM_WORLD, &send_req[0]);
    }
  }
  const auto end{std::chrono::time_point_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now())};

  return utility::ExerciseReturn<utility::Exercise::GAME_OF_LIFE_ASYNC>{
      .time{static_cast<uint64_t>((end - start).count())},
      .dims{specifics.dims[0], specifics.dims[1]},
  };
}
