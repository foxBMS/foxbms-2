#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# We kindly request you to use one or more of the following phrases to refer to
# foxBMS in your hardware, software, documentation or advertising materials:
#
# - "This product uses parts of foxBMS®"
# - "This product includes parts of foxBMS®"
# - "This product is derived from foxBMS®"

"""Script to plot time period in between signals"""

import argparse
import logging
import sys
from pathlib import Path
from typing import Dict, List

import matplotlib.pyplot as plt
import numpy as np


def extract_timestamps_for_ids(
    log_file: Path, target_ids: List[str]
) -> Dict[str, List[float]]:
    """Extracts the timestamps for each target id"""
    # Check if the file exists
    if not log_file.exists():
        logging.warning("The file '%s' does not exist.", log_file)
        sys.exit(1)
    timestamps = {target_id: [] for target_id in target_ids}
    for line in log_file.read_text(encoding="utf-8").splitlines():
        if "ID:" not in line:
            continue
        parts = line.split()
        # get the position of ID and the timestamps
        id_index = parts.index("ID:") + 1
        timestamp_index = parts.index("Timestamp:") + 1
        # Extract id and timestamp
        id_value = parts[id_index]
        timestamp_value = float(parts[timestamp_index])
        if id_value in target_ids:
            timestamps[id_value].append(timestamp_value)
    return timestamps


def plot_stats(difference: List[float], target_id: str, show: bool = False) -> None:
    """plots the periods along with statistics"""
    min_diff = min(difference)
    max_diff = max(difference)
    mean_diff = np.mean(difference)
    std_dev = np.std(difference)
    # Plot time differences
    fig = plt.figure()
    plt.plot(difference, marker=".", label=f"ID: {target_id}")
    plt.xlabel("Consecutive Signals")
    plt.ylabel("Time Difference (seconds)")
    plt.title("Time Difference Between Consecutive Signals")
    # Mark min and max values
    min_index = difference.index(min_diff)
    max_index = difference.index(max_diff)
    plt.scatter(
        min_index,
        min_diff,
        marker="v",
        color="red",
        label=f"Min: {min_diff:.3f}s",
        s=80,
    )
    plt.scatter(
        max_index,
        max_diff,
        marker="^",
        color="red",
        label=f"Max: {max_diff:.3f}s",
        s=80,
    )
    # Add dotted line for mean
    plt.axhline(
        y=mean_diff, color="red", linestyle="--", label=f"Mean: {mean_diff:.3f}s"
    )
    # Display std deviation in the legend
    legend_text = f"Std Dev: {std_dev:.3f}s"
    handles, labels = plt.gca().get_legend_handles_labels()
    handles.append(plt.Line2D([0], [0], color="none"))  # Dummy handle for space
    labels.append(legend_text)
    plt.legend(handles=handles, labels=labels)
    plt.grid(True)
    if show:
        plt.show()
    else:
        fig.savefig(f"{target_id}.png", dpi=fig.dpi)


def plot_time_differences_for_ids(
    log_file: Path, target_ids: List[str], show: bool = False
) -> None:
    """calls extract timestamp function and statistics plotting function"""
    timestamps = extract_timestamps_for_ids(log_file, target_ids)
    for target_id in target_ids:
        if not timestamps[target_id]:
            logging.warning("No timestamps found for the target id: %s", target_id)
            continue
        all_timestamps_from_id = timestamps.get(target_id)
        if not len(all_timestamps_from_id) > 1:
            logging.warning("Only a single occurrence for the target id: %s", target_id)
            continue
        differences = []
        # iterate over timestamps starting from the second element
        for i, timestamp in enumerate(all_timestamps_from_id[1:], start=1):
            previous_timestamp = all_timestamps_from_id[i - 1]
            difference = abs(timestamp - previous_timestamp)
            differences.append(round(difference, 3))
        # Calculate statistics
        plot_stats(differences, target_id, show=show)


def main() -> None:
    "Takes path and IDs are argument and calls the plot function"
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v",
        "--verbosity",
        dest="verbosity",
        action="count",
        default=1,
        help="set verbosity level",
    )
    # Add arguments for log file and target ids
    parser.add_argument("log_file", type=Path, help="Path to the log file")
    parser.add_argument("target_ids", type=str, nargs="+", help="List of target IDs")
    parser.add_argument(
        "--show", action="store_true", help="Show plots instead of saving them."
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    plot_time_differences_for_ids(args.log_file, args.target_ids, show=args.show)


if __name__ == "__main__":
    main()
