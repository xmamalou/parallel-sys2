# Parallel Systems Exercise Batch 1 -- Solutions to Batch 1 of Exercises for the Parallel
# Systems Course of the "Computer Engineering" Masters Programme of NKUA
# Copyright (C) 2025 Christoforos-Marios Mamaloukas

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import sys
import os

import matplotlib.pyplot as plt
import pandas as pd

# READ FILE
data_filepath = os.getcwd() + "/" + sys.argv[1]
data_file = open(data_filepath)

data = data_file.read()

data_file.close()

# PARSE CONTENT
exercise_data = {
    "time": [],
    "dims": [],
    "nodes": [],
}

data_array = data.split("[EXERCISE 2]")
for str in data_array[1:]:
    str = str.split("\n")
    exercise_data["nodes"].append(int(str[2].split(" = ")[1]))
    exercise_data["dims"].append(int(str[3].split(" = ")[1].split("x")[0]))
    exercise_data["time"].append(float(str[1].split(" = ")[1]))

# Assume that OMP = MPI
omp_dataframe = pd.DataFrame(exercise_data)
omp_dataframe = omp_dataframe.sort_values(by="nodes", ascending=True)
# we separate the dataframes by jobs, index is jobs-1
omp_dataframes = omp_dataframe.groupby("nodes")
omp_dataframes = [
    omp_dataframe.loc[omp_dataframe["nodes"] == i]
    for i in omp_dataframe["nodes"].unique()
]
omp_dataframes = [df.reset_index(drop=True) for df in omp_dataframes]
omp_dataframes = [df.drop(columns=["nodes"]) for df in omp_dataframes]
omp_dataframes = [
    df.sort_values(
        by="dims",
        ascending=True,
    )
    for df in omp_dataframes
]

# PLOTTING
# Plot the data (time vs throws) on its own plot
plt.figure(figsize=(10, 5))
plt.title("MPI - Game of Life")
plt.xscale("log")
plt.xlabel("Matrix Size")
plt.ylabel("Execution time (sec)")
for i in range(len(omp_dataframes)):
    color = plt.cm.viridis(i / len(omp_dataframes))
    plt.plot(
        omp_dataframes[i]["dims"],
        omp_dataframes[i]["time"] / 1000,
        marker="o",
        color=color,
    )
    # add legend
    plt.legend(
        [f"nodes = {i}" for i in [2, 4, 8, 16]],
        loc="upper left",
    )
plt.xticks(
    omp_dataframes[0]["dims"],
    [
        f"{omp_dataframes[0]['dims'][i]}x{omp_dataframes[0]['dims'][i]}"
        for i in range(len(omp_dataframes[0]["dims"]))
    ],
)
plt.grid()
plt.savefig("../artifacts/plot1_gol.png")
plt.close()
