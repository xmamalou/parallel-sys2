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

#include <iostream>
#include <string_view>
#include <vector>
#include <utility>
#include <charconv>
#include <algorithm>

#include <stdint.h>

#include "exercises.hxx"
#include "types.hxx"

// --- FUNCTION DECLARATIONS --- //

auto read_args(int argc, char** argv) -> utility::Options;

// Commands

auto print_help(const utility::Options&) -> void;

// --- FUNCTION DEFINITIONS --- //

auto main(int argc, char** argv) -> int {
  const auto options = read_args(argc, argv);

  const auto& command = reinterpret_cast<utility::Command>(options.command_p);
  command(options);

  return 0;
}

auto read_args(int argc, char** argv) -> utility::Options {
  using StringVector = std::vector<std::string_view>;
  StringVector arguments{ {std::string_view(argv[0])} };

  for (uint32_t i{ 1 }; i < static_cast<uint32_t>(argc); i++) {
      arguments.push_back(std::string_view(argv[i]));
  }

  utility::Options options{
    .command_p{ reinterpret_cast<void*>(print_help) },
    .appname{ std::move(arguments[0]) },
  };
  if (arguments.size() >= 2) {
    if (arguments[1] == "help") {
      // Help does not use any other options other than the app name
      // so we do not need to do anything else. Also, we have already
      // set the print_help function as the command to run, so we don't
      // need to set it again
      return options;
    } else if (arguments[1] == "run") {
      if (arguments.size() < 3) {
        std::cout << "\x1b[31mERROR!! `run` has one mandatory parameter (exercise number)!\x1b[0m\n";
        return options; // print_help is already set as the command to run
      }

      auto& exe_num_str{ arguments[2] };
      uint32_t exe_num{ 0 };
      auto result = std::from_chars(
                                    exe_num_str.data(),
                                    exe_num_str.data() + exe_num_str.size(),
                                    exe_num);
      if (result.ec == std::errc::invalid_argument ||
          (exe_num > 4 || exe_num < 1)) {
        std::cout << "\x1b[31mERROR!! `run` has received an invalid argument: " << exe_num_str << "! This is not an integer from 1 to 4!\x1b[0m\n";
        return options; // print_help is already set as the command to run
      }

      options.exe = static_cast<utility::Exercise>(exe_num);
      switch (options.exe) {
        case utility::Exercise::PI_MONTE_CARLO:
          options.command_p = reinterpret_cast<void*>(exe::pi_monte_carlo);
          break;
        case utility::Exercise::GAME_OF_LIFE:
          options.command_p = reinterpret_cast<void*>(exe::game_of_life);
          break;
        case utility::Exercise::GAME_OF_LIFE_ASYNC:
          options.command_p = reinterpret_cast<void*>(exe::game_of_life_async);
          break;
        case utility::Exercise::GAME_OF_LIFE_MIX:
          options.command_p = reinterpret_cast<void*>(exe::game_of_life_mix);
          break;
        default:
          options.command_p = reinterpret_cast<void*>(print_help);
          break;
      }

    } else {
      std::cout << "\x1b[31mERROR!! " << options.appname << " has received an invalid command: " << arguments[1] << "!\x1b[0m\n";
    }
  }

  return options;
}

auto print_help(const utility::Options& options) -> void {
  std::cout << "\x1b[33m" <<
    R"(Parallel Systems Project 2 -- Christoforos-Marios Mamaloukas -- Parallel Systems Postgraduate Course -- NKUA
-------------------------------------------------------------------------------------------------------------
USAGE: )" << options.appname << R"( <COMMAND> [(<OPTION> <VALUE>).. | <FLAG>..]
                                    
--- AVAILABLE COMMANDS ---
help: Print this message
run <number>: Execute the <number>th exercise (where <number> an integer from 1 to 4)
                                    
--- AVAILABLE OPTIONS ---
-> [global]
  * --nodes <number> | -n <number> : The amount of nodes to run the algo on

-> run
1 (PI_MONTE_CARLO)
  * --throws <number> | -t <number> : The amount of throws to perform

2 (GAME_OF_LIFE)
  * --generations <number> | -g <number> : The generations to run the game of life for
  * --matrix <number>x<number> | -m <number>x<number> : The size of the matrix for the game

3 (GAME_OF_LIFE_ASYNC)
  * --generations <number> | -g <number> : The generations to run the game of life for
  * --matrix <number>x<number> | -m <number>x<number> : The size of the matrix for the game

4 (GAME_OF_LIFE_MIX)
  * --generations <number> | -g <number> : The generations to run the game of life for
  * --matrix <number>x<number> | -m <number>x<number> : The size of the matrix for the game

--- AVAILABLE FLAGS ---
(There are no available flags for this exercise)
)" << "\x1b[0m";

  return;
}
