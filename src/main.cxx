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

#include <charconv>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

#include <stdint.h>

#include "mpi.h"

#include "exercises.hxx"
#include "types.hxx"

// --- TYPES --- //

enum class ArgumentType {
  OPTION,
  FLAG,
};

struct Argument {
  std::string_view long_name{"--"};
  std::string_view short_name{"-"};
  char option{'?'};
  ArgumentType type{ArgumentType::OPTION};
};

using PiMCReturn = utility::ExerciseReturn<utility::Exercise::PI_MONTE_CARLO>;
using GoLReturn = utility::ExerciseReturn<utility::Exercise::GAME_OF_LIFE>;
using GoLAsyncReturn =
    utility::ExerciseReturn<utility::Exercise::GAME_OF_LIFE_ASYNC>;
using GoLMixReturn =
    utility::ExerciseReturn<utility::Exercise::GAME_OF_LIFE_MIX>;
using HelpReturn = utility::ExerciseReturn<utility::Exercise::HELP>;

// --- GLOBALS --- //

const std::vector<Argument> arguments_g{
    Argument{"--file", "-f", 'f', ArgumentType::OPTION},
    Argument{"--throws", "-t", 't', ArgumentType::OPTION},
    Argument{"--generations", "-g", 'g', ArgumentType::OPTION},
    Argument{"--dimensions", "-d", 'd', ArgumentType::OPTION},
    Argument{"--jobs", "-j", 'j', ArgumentType::OPTION},
    Argument{"-Fclear", "-Fc", 'c', ArgumentType::FLAG},
};

// --- FUNCTION DECLARATIONS --- //

auto read_args(int argc, char **argv) -> utility::Options;
auto get_option(std::string_view argument) -> char;

// Commands

auto print_help(const utility::Options &) -> utility::ExerciseVariantReturn;

// --- FUNCTION DEFINITIONS --- //

auto main(int argc, char **argv) -> int {
  MPI_Init(nullptr, nullptr);
  int32_t rank{0};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  try {
    const auto options = read_args(argc, argv);

    const auto &command = reinterpret_cast<utility::Command>(options.command_p);
    auto return_value = command(options);
    // The 0th process manages the results here
    if (rank == 0) {
      std::visit(
          [&](auto &value) {
            using return_t = std::decay_t<decltype(value)>;

            std::ofstream file(
                options.filepath.data(),
                (options.do_append ? std::ios_base::app : std::ios::ate));
            if constexpr (std::is_same_v<return_t, PiMCReturn>) {
              std::cout << "π is " << value.pi << "! This took " << value.time
                        << "msec to calculate!";
              file << "[EXERCISE 1]\nπ = " << value.pi
                   << "\ntime = " << value.time << "\n";
            } else if constexpr (!std::is_same_v<return_t, HelpReturn>) {
              auto exe_num{2};
              if constexpr (std::is_same_v<return_t, GoLAsyncReturn>)
                exe_num = 3;
              else if constexpr (std::is_same_v<return_t, GoLMixReturn>)
                exe_num = 4;

              std::cout << "Finished with the Game of Life! This took "
                        << value.time << "msec to run!";
              file << "[EXERCISE " << exe_num << "]\ntime = " << value.time
                   << "\ndimensions = " << value.dims[0] << "x" << value.dims[1]
                   << "\n";
            }
            file.close();
          },
          return_value);
    }
  } catch (utility::Error err) {
    if (rank == 0) {
      switch (err.error) {
      case utility::Error::ErrorCode::WRONG_COMMAND_ERR:
        std::cerr << "\x1b[31mERROR!! Command " << err.erroneous
                  << " is unrecognized!\x1b[0m\n";
        break;
      case utility::Error::ErrorCode::NO_VALUE_ERR:
        std::cerr << "\x1b[31mERROR!! No value given for " << err.erroneous
                  << "!\x1b[0m\n";
        break;
      case utility::Error::ErrorCode::BAD_VALUE_ERR:
        std::cerr << "\x1b[31mERROR!! " << err.erroneous
                  << " has received a wrong value!\x1b[0m\n";
        break;
      case utility::Error::ErrorCode::SHOW_HELP:
        break;
      default:
        std::cerr << "\x1b[31mERROR!! ERROR!! ERROR!!\x1b[0m\n";
      }

      const auto options = utility::Options{
          .appname{argv[0]},
      };
      print_help(options);
    }
  }

  MPI_Finalize();
  return 0;
}

auto read_args(int argc, char **argv) -> utility::Options {
  using StringVector = std::vector<std::string_view>;
  StringVector arguments{{std::string_view(argv[0])}};

  for (uint32_t i{1}; i < static_cast<uint32_t>(argc); i++) {
    arguments.push_back(std::string_view(argv[i]));
  }

  utility::Options options{
      .command_p{reinterpret_cast<void *>(print_help)},
      .appname{std::move(arguments[0])},
  };
  if (arguments.size() >= 2) {
    if (arguments[1] == "help")
      throw utility::Error{"", utility::Error::ErrorCode::SHOW_HELP};
    else if (arguments[1] == "run") {
      if (arguments.size() < 3)
        throw utility::Error{"run", utility::Error::ErrorCode::NO_VALUE_ERR};

      auto &exe_num_str{arguments[2]};
      uint32_t exe_num{0};
      auto result = std::from_chars(
          exe_num_str.data(), exe_num_str.data() + exe_num_str.size(), exe_num);
      if (result.ec == std::errc::invalid_argument ||
          (exe_num > 4 || exe_num < 1))
        throw utility::Error{"run", utility::Error::ErrorCode::BAD_VALUE_ERR};

      options.exe = static_cast<utility::Exercise>(exe_num);
      switch (options.exe) {
      case utility::Exercise::PI_MONTE_CARLO: {
        options.command_p = reinterpret_cast<void *>(exe::pi_monte_carlo);
        auto pi_opts =
            utility::ExerciseOptions<utility::Exercise::PI_MONTE_CARLO>{};
        for (auto it{arguments.begin() + 3}; it < arguments.end(); it++) {
          auto option = get_option(*it);
          if (option == '\0')
            continue;
          switch (option) {
          case 't':
            if (it++ == arguments.end())
              throw utility::Error{"--throw / -t",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            pi_opts.throws = std::atoll((*it).data());
            break;
          case 'f':
            if (it++ == arguments.end())
              throw utility::Error{"--file / -f",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            options.filepath = *it;
            break;
          case 'c':
            options.do_append = false;
          }
        }
        options.specifics = std::move(pi_opts);
        break;
      }
      case utility::Exercise::GAME_OF_LIFE: {
        options.command_p = reinterpret_cast<void *>(exe::game_of_life);
        auto gol_opts =
            utility::ExerciseOptions<utility::Exercise::GAME_OF_LIFE>{};
        for (auto it{arguments.begin() + 3}; it < arguments.end(); it++) {
          auto option = get_option(*it);
          if (option == '\0')
            continue;
          switch (option) {
          case 'g': {
            if (it++ == arguments.end())
              throw utility::Error{"--generations / -g",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            gol_opts.generations = std::atoll((*it).data());
            break;
          }
          case 'd': {
            if (it++ == arguments.end())
              throw utility::Error{"--dimensions / -g",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            auto dims = std::stringstream((*it).data());
            dims >> gol_opts.dims[0];
            dims.ignore(1, 'x');
            dims >> gol_opts.dims[1];
            break;
          }
          case 'f': {
            if (it++ == arguments.end())
              throw utility::Error{"--file / -f",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            options.filepath = *it;
            break;
          }
          case 'c': {
            options.do_append = false;
          }
          }
        }
        options.specifics = std::move(gol_opts);
        break;
      }
      case utility::Exercise::GAME_OF_LIFE_ASYNC: {
        options.command_p = reinterpret_cast<void *>(exe::game_of_life_async);
        auto gol_opts =
            utility::ExerciseOptions<utility::Exercise::GAME_OF_LIFE_ASYNC>{};
        for (auto it{arguments.begin() + 3}; it < arguments.end(); it++) {
          auto option = get_option(*it);
          if (option == '\0')
            continue;
          switch (option) {
          case 'g': {
            if (it++ == arguments.end())
              throw utility::Error{"--generations / -g",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            gol_opts.generations = std::atoll((*it).data());
            break;
          }
          case 'd': {
            if (it++ == arguments.end())
              throw utility::Error{"--dimensions / -g",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            auto dims = std::stringstream((*it).data());
            dims >> gol_opts.dims[0];
            dims.ignore(1, 'x');
            dims >> gol_opts.dims[1];
            break;
          }
          case 'f': {
            if (it++ == arguments.end())
              throw utility::Error{"--file / -f",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            options.filepath = *it;
            break;
          }
          case 'c': {
            options.do_append = false;
          }
          }
        }
        options.specifics = std::move(gol_opts);
        break;
      }
      case utility::Exercise::GAME_OF_LIFE_MIX: {
        options.command_p = reinterpret_cast<void *>(exe::game_of_life_mix);
        auto gol_opts =
            utility::ExerciseOptions<utility::Exercise::GAME_OF_LIFE_MIX>{};
        for (auto it{arguments.begin() + 3}; it < arguments.end(); it++) {
          auto option = get_option(*it);
          if (option == '\0')
            continue;
          switch (option) {
          case 'g': {
            if (it++ == arguments.end())
              throw utility::Error{"--generations / -g",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            gol_opts.generations = std::atoll((*it).data());
            break;
          }
          case 'd': {
            if (it++ == arguments.end())
              throw utility::Error{"--dimensions / -g",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            auto dims = std::stringstream((*it).data());
            dims >> gol_opts.dims[0];
            dims.ignore(1, 'x');
            dims >> gol_opts.dims[1];
            break;
          }
          case 'j': {
            if (it++ == arguments.end())
              throw utility::Error{"--jobs / -j",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            gol_opts.jobs = std::atoi((*it).data());
            break;
          }
          case 'f': {
            if (it++ == arguments.end())
              throw utility::Error{"--file / -f",
                                   utility::Error::ErrorCode::NO_VALUE_ERR};
            options.filepath = *it;
            break;
          }
          case 'c': {
            options.do_append = false;
          }
          }
        }
        options.specifics = std::move(gol_opts);
        break;
      }
      default:
        options.command_p = reinterpret_cast<void *>(print_help);
        break;
      }

    } else
      throw utility::Error{arguments[1],
                           utility::Error::ErrorCode::WRONG_COMMAND_ERR};
  }

  return options;
}

inline auto get_option(std::string_view argument) -> char {
  for (const auto &arg : arguments_g) {
    if (arg.long_name == argument || arg.short_name == argument)
      return arg.option;
  }

  return '\0';
}

inline auto print_help(const utility::Options &options)
    -> utility::ExerciseVariantReturn {
  std::cout << "\x1b[33m" <<
      R"(Parallel Systems Project 2 -- Christoforos-Marios Mamaloukas -- Parallel Systems Postgraduate Course -- NKUA
-------------------------------------------------------------------------------------------------------------
USAGE: )" << options.appname
            << R"( <COMMAND> [(<OPTION> <VALUE>).. | <FLAG>..]

--- AVAILABLE COMMANDS ---
help: Print this message
run <number>: Execute the <number>th exercise (where <number> an integer from 1 to 4)
                                    
--- AVAILABLE OPTIONS ---
-> [global]
  * --file <path> | -f <path> : The path to save data in [DEFAULT: './out.txt']

-> run
1 (PI_MONTE_CARLO)
  * --throws <number> | -t <number> : The amount of throws to perform [DEFAULT: 1]

2 (GAME_OF_LIFE)
  * --generations <number> | -g <number> : The generations to run the game of life for [DEFAULT: 1]
  * --dimensions <number>x<number> | -d <number>x<number> : The size of the matrix for the game [DEFAULT: 2x2]

3 (GAME_OF_LIFE_ASYNC)
  * --generations <number> | -g <number> : The generations to run the game of life for [DEFAULT: 1]
  * --dimensions <number>x<number> | -d <number>x<number> : The size of the matrix for the game [DEFAULT: 2x2]

4 (GAME_OF_LIFE_MIX)
  * --generations <number> | -g <number> : The generations to run the game of life for [DEFAULT: 1]
  * --dimensions <number>x<number> | -d <number>x<number> : The size of the matrix for the game [DEFAULT: 2x2]
  * --jobs <number> | -j <number> : The amount of jobs to use per node [DEFAULT: 1]

--- AVAILABLE FLAGS ---
-> [global]
  * -Fclear | -Fc : If this flag is set, the file specified by `--file <path>` (or the default one) is 
    overwritten instead of having data appended to. [DEFAULT: data is appended to file]
)" << "\x1b[0m";

  return utility::ExerciseReturn<utility::Exercise::HELP>{};
}
