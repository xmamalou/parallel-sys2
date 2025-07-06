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

// --- FUNCTION DECLARATIONS --- //

auto gol_check(utility::Matrix<short> &neighborhood, const uint32_t at_x,
               const uint32_t at_y) -> bool;

// --- FUNCTION DEFINITIONS --- //

auto exe::game_of_life(const utility::Options &options)
    -> utility::ExerciseVariantReturn {
  int32_t rank{0};
  int32_t nodes{0};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nodes);

  const auto specifics{
      std::get<utility::ExerciseOptions<utility::Exercise::GAME_OF_LIFE>>(
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
    for (uint32_t i{0}; i < matrix.size<0>(); i++) {
      for (uint32_t j{0}; j < matrix.size<1>(); j++) {
        const auto time{std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now())};
        const auto duration{time.time_since_epoch().count()};
        std::srand(duration);

        auto value{static_cast<double>(std::rand()) /
                   static_cast<double>(RAND_MAX)};

        matrix(i, j) = static_cast<short>(value <= 0.5);
      }
    }
  }

  for (uint32_t i{0}; i < specifics.generations; i++) {

    MPI_Bcast(reinterpret_cast<void *>(const_cast<short *>(matrix.data())),
              matrix.size<0>() * matrix.size<1>(), MPI_SHORT, 0,
              MPI_COMM_WORLD);

    // we want the memory each process will get to be continguous
    const auto my_elem_count{matrix.size<0>() * matrix.size<1>() / nodes};
    const auto my_top{
        rank == 0 ? 0 : (rank - 1) * my_elem_count / matrix.size<0>()};
    const auto my_right{
        rank == 0 ? 0 : (rank - 1) * my_elem_count % matrix.size<0>()};

    for (uint32_t i{0}; i < my_elem_count; i++) {
      auto x{(my_right + i) % matrix.size<0>()},
          y{(my_right + i) / matrix.size<0>()};
      matrix(x, y) = static_cast<short>(gol_check(matrix, x, y));
    }

    if (rank != 0) {
      MPI_Gather(reinterpret_cast<void *>(&matrix(my_right, my_top)),
                 my_elem_count, MPI_SHORT, nullptr, 0, MPI_SHORT, 0,
                 MPI_COMM_WORLD);
    } else {
      MPI_Gather(nullptr, 0, MPI_SHORT,
                 reinterpret_cast<void *>(&matrix(my_right, my_top)),
                 my_elem_count, MPI_SHORT, 0, MPI_COMM_WORLD);
    }
  }
  const auto end{std::chrono::time_point_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now())};

  return utility::ExerciseReturn<utility::Exercise::GAME_OF_LIFE>{
      .time{static_cast<uint64_t>((end - start).count())},
      .dims{specifics.dims[0], specifics.dims[1]},
  };
}

inline auto gol_check(utility::Matrix<short> &neighborhood, const uint32_t at_x,
                      const uint32_t at_y) -> bool {
  const auto right{at_x > 0 ? at_x - 1 : at_x};
  const auto top{at_y > 0 ? at_y - 1 : at_y};
  const auto left{at_x < neighborhood.size<0>() - 1 ? at_x + 1 : at_x};
  const auto bottom{at_y < neighborhood.size<1>() - 1 ? at_y + 1 : at_y};

  uint32_t alive_neighborhoods{0};
  for (uint32_t i{right}; i <= left; i++) {
    for (uint32_t j{top}; j <= bottom; j++) {
      if (i == at_x || j == at_y)
        continue;

      if (neighborhood(i, j) == 1)
        alive_neighborhoods++;
    }
  }

  switch (alive_neighborhoods) {
  case 3:
    return true;
  case 2:
    return static_cast<bool>(neighborhood(at_x, at_y));
  default:
    return false;
  }
}
