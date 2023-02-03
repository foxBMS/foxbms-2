# CRC functions

These Jupyter notebooks help to generate the CRC-functions as
fast table implementations that are used by certain monitoring
ICs.

## Installation

In order to use these notebooks you need to have Jupyter notebooks and crcmod
installed. They come with the conda environment shipped in this repository.

## Usage

Activate your conda environment and then go to this directory (this makes
finding the notebooks easier). Then start Jupyter notebook with:

    Jupyter notebook

Alternatively just execute `crc.bat` which is a wrapper script for jupyter.
This should open your browser with a Jupyter notebook instance. If not, observe
console output, there should be a link to open.

In the jupyter notebook browser you can select a notebook and then open it.
When you click on restart and run all cells it will execute the whole notebook.

## CRC-1 - 0x48F

For details of the CRC polynomial see [here](./crc-10_0x48f.md).

## CRC-15 - 0xC599

For details of the CRC polynomial see [here](./crc-15_0xc599.md).
