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

#ifndef _EXERCISES_HXX_
#define _EXERCISES_HXX_

#include "types.hxx"

namespace exe {
  auto pi_monte_carlo(const utility::Options&) -> void;
  auto game_of_life(const utility::Options&) -> void;
  auto game_of_life_async(const utility::Options&) -> void;
  auto game_of_life_mix(const utility::Options&) -> void;
}

#endif /* _EXERCISES_HXX_ */
