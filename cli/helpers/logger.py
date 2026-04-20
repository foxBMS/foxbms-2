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

"""Logging configuration for the foxBMS tool suite.

This module provides custom logging functionality, including:

- Initialization and configuration of a module-level logger,
- A utility to suppress verbose logging from selected third-party modules,
- A function to set the global logging level and format according to
  user-defined verbosity.

Attributes:
    DISABLE_LOGGING_FOR_MODULES: List of third-party modules for which logging
        output should be suppressed except for critical errors.
    logger: The main logger instance for the foxBMS tool suite.

"""

# we need to import it in this wrapper, so that all other parts can make use of
# the logger implement here
import logging  # noqa: TID251
from logging.handlers import QueueHandler, QueueListener  # noqa: TID251
from multiprocessing import Queue

from . import TOOL

DISABLE_LOGGING_FOR_MODULES = ["git", "can"]

logger = logging.getLogger(TOOL)


def ignore_third_party_logging() -> None:
    """Disable logging output for selected third-party modules except for
    critical messages.

    This helps to reduce log clutter from dependencies such as 'git' and 'can',
    allowing only critical errors to be shown from these sources.
    """
    for module in DISABLE_LOGGING_FOR_MODULES:
        logging.getLogger(module).setLevel(logging.CRITICAL)


def set_logging_level(
    verbosity: int = 1,
    _format: str = "%(asctime)s %(pathname)-9s:%(lineno)-4s %(levelname)-8s %(message)s",
    datefmt: str | None = None,
) -> None:
    """Configure the logging level and format for the module.

    Sets the logger level and formatter based on the provided verbosity and
    format.
    Ensures only a single handler is attached to prevent duplicate log entries.

    Args:
        verbosity: Verbosity level (1: WARNING, 2: INFO, 3: DEBUG).
        _format: Logging format style.
        datefmt: Date format style.

    """
    if verbosity < 1:
        verbosity = 1
    elif verbosity > 3:
        verbosity = 3

    logging_levels = {
        "1": logging.WARNING,
        "2": logging.INFO,
        "3": logging.DEBUG,
    }
    level = logging_levels[str(verbosity)]

    # Create a custom logger for your module
    logger.setLevel(level)

    # Create handler and formatter
    handler = logging.StreamHandler()
    formatter = logging.Formatter(_format, datefmt=datefmt)
    handler.setFormatter(formatter)

    # Avoid adding multiple handlers if this code runs multiple times
    if not logger.hasHandlers():
        logger.addHandler(handler)

    logger.debug("Setting logging level to %s", level)


def add_queue_handler(queue: Queue) -> None:
    """Adds a QueueHandler to the logger to enable logging
    via a multiprocessing queue.

    Args:
        queue: The multiprocessing queue to be used for
            logging messages.
    """
    queue_handler = QueueHandler(queue)
    logger.addHandler(queue_handler)


def get_listener(queue: Queue) -> QueueListener:
    """Creates and returns a QueueListener for handling log records
    from a queue.

    Args:
        queue: The multiprocessing queue from which to listen
            for log records.

    Returns:
        QueueListener: An instance of QueueListener attached to the
            provided queue and current logger handlers.
    """
    return QueueListener(queue, *logger.handlers)
