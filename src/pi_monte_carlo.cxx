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
#include <cmath>
#include <functional>
#include <iostream>
#include <random>

#include "mpi.h"

using MCOptions = utility::ExerciseOptions<utility::Exercise::PI_MONTE_CARLO>;

auto exe::pi_monte_carlo(const utility::Options &options)
    -> utility::ExerciseVariantReturn {
    auto specifics{
        std::get<utility::ExerciseOptions<utility::Exercise::PI_MONTE_CARLO>>(
            options.specifics)};
    int32_t nodes{0};
    int32_t rank{0};
    MPI_Comm_size(MPI_COMM_WORLD, &nodes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        std::cout << R"(--- CALCULATE PI WITH MONTE CARLO METHOD ---
-> Number of nodes: )"
                  << nodes << R"(
-> Number of total throws: )"
                  << specifics.throws << R"(
-> Data is saved in: )"
                  << options.filepath << R"(
---------------------------------------------
)";
    }

    auto time{std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now())};
    auto duration{time.time_since_epoch().count()};
    std::srand(1000 * rank + duration);

    uint64_t my_throws{specifics.throws / static_cast<uint32_t>(nodes)};
    uint64_t my_succ_throws{0};
    uint64_t final_succ_throws{0};
    double pi{0.0};
    for (uint32_t i{0}; i <= my_throws; i++) {
        double point[2]{
            static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX),
            static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX)};
        // std::cout << "(" << point[0] << ", " << point[1] << ")\n";
        if (std::pow(point[0], 2) + std::pow(point[1], 2) <= 1.0) {
            my_succ_throws++;
        }
    }

    MPI_Reduce(reinterpret_cast<void *>(&my_succ_throws),
               reinterpret_cast<void *>(&final_succ_throws), 1,
               MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        pi = 4.0 * static_cast<double>(final_succ_throws) /
             static_cast<double>(specifics.throws);
        auto end{std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now())};
        duration = (end - time).count();
    }

    return utility::ExerciseReturn<utility::Exercise::PI_MONTE_CARLO>{
        .pi{pi},
        .time{static_cast<uint64_t>(duration)},
    };
}
