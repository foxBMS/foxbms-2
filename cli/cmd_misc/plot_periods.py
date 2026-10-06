#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.lines import Line2D

from ..helpers.logger import logger


def extract_timestamps_for_ids(
    log_file: Path, target_ids: list[str]
) -> dict[str, list[float]]:
    """Extract the timestamps for each target id from the two possible formats."""
    if not log_file.exists():
        logger.warning("The file '%s' does not exist.", log_file)
        msg = f"{log_file} not found"
        raise FileNotFoundError(msg)

    timestamps: dict[str, list[float]] = {tid: [] for tid in target_ids}

    with log_file.open(encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith(";"):
                continue

            id_val, ts = _parse_line(line)
            if id_val is not None and ts is not None and id_val in timestamps:
                timestamps[id_val].append(ts)

    return timestamps


def _parse_line(line: str) -> tuple[str, float] | tuple[None, None]:
    """Parse a single log line and return (id, timestamp) or (None, None)."""
    parts = line.split()

    # Format: ID: <id> Timestamp: <ts>
    if "ID:" in line and "Timestamp:" in line:
        try:
            id_val = parts[parts.index("ID:") + 1]
            ts = float(parts[parts.index("Timestamp:") + 1])
        except (ValueError, IndexError):
            return None, None
        return id_val, ts
    # TRC format: <Num> <Time(ms)> DT <Bus> <IDhex> Rx/Tx ...
    if len(parts) >= 5:
        try:
            ts = float(parts[1]) / 1000
            return parts[4], ts
        except ValueError:
            pass

    return None, None


def plot_stats(
    differences: list[float],
    target_id: str,
    show: bool = False,
    output_path: Path = Path(),
) -> None:
    """Plot the periods along with statistics."""
    diff_array = np.array(differences)
    min_diff = diff_array.min()
    max_diff = diff_array.max()
    mean_diff = diff_array.mean()
    std_dev = diff_array.std()
    min_index = int(diff_array.argmin())
    max_index = int(diff_array.argmax())

    fig, ax = plt.subplots()
    ax.plot(differences, marker=".", label=f"ID: {target_id}")
    ax.set_xlabel("Consecutive Signals")
    ax.set_ylabel("Time Difference (seconds)")
    ax.set_title("Time Difference Between Consecutive Signals")

    ax.scatter(
        min_index,
        min_diff,
        marker="v",
        color="red",
        label=f"Min: {min_diff:.3f}s",
        s=80,
    )
    ax.scatter(
        max_index,
        max_diff,
        marker="^",
        color="red",
        label=f"Max: {max_diff:.3f}s",
        s=80,
    )
    ax.axhline(
        y=mean_diff, color="red", linestyle="--", label=f"Mean: {mean_diff:.3f}s"
    )

    handles, labels = ax.get_legend_handles_labels()
    handles.append(Line2D([0], [0], color="none"))
    labels.append(f"Std Dev: {std_dev:.3f}s")
    ax.legend(handles=handles, labels=labels)
    ax.grid(True)

    if show:
        plt.show()
    else:
        output_path.mkdir(parents=True, exist_ok=True)
        fig.savefig(output_path / f"{target_id}.png", dpi=fig.dpi)
        logger.info("Plot saved in %s", output_path.resolve())
    plt.close(fig)


def plot_time_differences_for_ids(
    log_file: Path,
    target_ids: list[str],
    show: bool = False,
    output_path: Path = Path(),
) -> None:
    """Call extract timestamp function and statistics plotting function."""
    timestamps = extract_timestamps_for_ids(log_file, target_ids)
    for target_id in target_ids:
        ts_list = timestamps[target_id]
        if not ts_list:
            logger.warning("No timestamps found for the target id: %s", target_id)
            continue
        if len(ts_list) <= 1:
            logger.warning("Only a single occurrence for the target id: %s", target_id)
            continue
        ts_array = np.array(ts_list)
        differences = np.abs(np.diff(ts_array)).round(3).tolist()
        plot_stats(differences, target_id, show=show, output_path=output_path)


def run_plot_periods(
    log_file: Path,
    target_ids: list[str],
    show: bool = False,
    output_path: Path = Path(),
) -> int:
    """Run the plot software."""
    try:
        plot_time_differences_for_ids(
            log_file, target_ids, show=show, output_path=output_path
        )
    except FileNotFoundError:
        return 1
    return 0
