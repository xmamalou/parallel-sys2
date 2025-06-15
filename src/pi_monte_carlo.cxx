/*
    Parallel Systems Exercise Batch 2 -- Solutions to Batch 2 of Exercises for the Parallel
    Systems Course of the "Computer Engineering" Masters Programme of NKUA
    Copyright (C) 2025 Christoforos-Marios Mamaloukas

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

#include <iostream>
#include <cmath>

#include "mpi.h"

auto exe::pi_monte_carlo(const utility::Options& options) -> void {
    int32_t rank{ 0 };
    MPI_Comm_rank(
        MPI_COMM_WORLD,
        &rank);

    if ( rank == 0 ) {
        std::cout << R"(
--- CALCULATE PI WITH MONTE CARLO METHOD ---
-> Number of nodes: )" << options.nodes << R"(
-> Number of total throws: )" << options.throws << R"(
---------------------------------------------
)";
    }

    uint64_t my_throws{ options.throws/static_cast<uint32_t>(rank) };
    uint64_t my_succ_throws{ 0 };
    for (uint32_t i{ 0 }; i <= my_throws; i++) {
        double point[2]{
          static_cast<double>(rand())/static_cast<double>(RAND_MAX),
          static_cast<double>(rand())/static_cast<double>(RAND_MAX) };
        std::cout << "(" << point[0] << ", " << point[1] << ")";
        if ( std::sqrt(std::pow(point[0], 2) + std::pow(point[1], 2)) <= 1.0 ) {
            my_succ_throws++;
        }
    }

    std::cout << "Your successful throws are " << my_succ_throws << "\n";

    uint64_t n{ 0 };
    if (rank == 0) {
        uint64_t received_throws{ 0 };
        while (1 + std::pow(2.0, static_cast<float>(n)) < options.nodes) {
            MPI_Status status;
            MPI_Recv(
                reinterpret_cast<void*>(&received_throws),
                1,
                MPI_UNSIGNED_LONG_LONG,
                1,
                0,
                MPI_COMM_WORLD,
                &status);
        };
        my_succ_throws += received_throws;

        std::cout << "Total successful throws are: " << my_succ_throws << "\n";
    } else {
        MPI_Send(
            reinterpret_cast<void*>(&my_succ_throws),
            1,
            MPI_UNSIGNED_LONG_LONG,
            rank - 1,
            0,
            MPI_COMM_WORLD);
        while (1 + static_cast<uint32_t>(std::pow(2.0, static_cast<float>(n))) < options.nodes) {
            std::cout << "Currently at n = " << n << "\n";
            auto dest_rank{ rank - static_cast<uint32_t>(std::pow(2.0, static_cast<float>(n))) >= 0
              ? rank - static_cast<uint32_t>(std::pow(2.0, static_cast<float>(n)))
              : 0};
            auto source_rank{ rank + static_cast<uint32_t>(std::pow(2.0, static_cast<float>(n))) < options.nodes
              ? rank + static_cast<uint32_t>(std::pow(2.0, static_cast<float>(n)))
              : options.nodes - 1};

            uint64_t received_throws{ 0 };
            MPI_Status status;
            MPI_Recv(
                reinterpret_cast<void*>(&received_throws),
                1,
                MPI_UNSIGNED_LONG_LONG,
                source_rank,
                0,
                MPI_COMM_WORLD,
                &status);
            my_succ_throws += received_throws;
            MPI_Send(
                reinterpret_cast<void*>(&my_succ_throws),
                1,
                MPI_UNSIGNED_LONG_LONG,
                dest_rank,
                0,
                MPI_COMM_WORLD);
        }
    }

    return;
}
