.. include:: ./../../../macros.txt

.. _FOX_PLOT:

plot
====

Implements a cli tool to plot measured data.

This tool extracts data from files and plots it as a line graph according to a
given specification.

.. figure:: ./../../../../build/docs/example_data/cell_voltage_current.png
    :alt: Example Graph - Cell Voltage, Current
    :name: cell-voltage-current
    :width: 65%
    :align: center

    Example Graph - Cell Voltage, Current

Usage
-----

.. include:: ./../../../../build/docs/fox_plot_help.txt

The input-data can contain files and directories.
If the data-type has been specified in the command the given files will be
processed accordingly and only files with the given data-type will be read from
the given directories.

.. note::
    If the input-data contains a directory, the data-type has to be specified.

If no output-directory is specified, a directory by the name
``<year-month-dayThour_minute_second.millisecond>``
is created in ``foxbms-2/build/plots`` and set as the output-directory.

For each input-file a directory by the same name will be created in the
output-directory in which the plots will be saved.

Configuration Files
-------------------

Two configuration files are needed to properly extract the data from the
input-files and then plot this data.
These files have to be yaml-files and need to have a certain structure to work
with the tool, which will be explained in the following sections.

Data Configuration File
^^^^^^^^^^^^^^^^^^^^^^^

The data is extracted from the input-files according to the specifications in
the Data Configuration File and then saved as a parquet file with the same name
as the original input-file in a sub-directory.
When re-plotting the already extracted data only the parquet file is accessed.

.. note::
    If the Data Configuration file or input-file have been changed after
    plotting it at least once, the input-data will not be extracted again.
    The parquet file has to be deleted before the data can be plotted with the
    new configuration.

Currently only input-files in the csv-format can be read by the tool.
For such files the Data Configuration File needs to have the following
structure:

- ``general``:

    - ``skip``: Specifies how many lines have to be skipped at the beginning of
      the file.
    - ``precision``: Specifies the precision of float values

- ``columns``: List that contains all names of the columns to be extracted from
  the input-files with the corresponding value-type.

For each column, the following data types are supported: ``string``, ``float``,
``int``, ``datetime``.

The following shows an example configuration.

.. literalinclude:: img/csv_config.yaml
    :language: yaml
    :caption: Example Data Configuration for csv-input-files
    :name: csv-data-configuration-file

Plot Configuration File
^^^^^^^^^^^^^^^^^^^^^^^

The plot tool can currently only plot lines.

Several plots can be created with a Plot Configuration File
and a plot can contain up to three line-graphs.

.. note::
    The plot configuration of the example plot displayed at the beginning is
    shown below as a reference,
    with only the Cell Voltage line given for simplicity.

The Plot Configuration File has to contain one block for each plot.
The key of a plot has to contain the plot-type.
If the Configuration File contains more than one plot the keys have to be
numbered with an underscore separating the type from the number.

The block of a plot has four main components:

- ``name``: The name with which the plot is saved in the end.
- ``mapping``: Block that contains the configuration of each graph.

    - ``x``: Specifies which column for the x-axis
    - ``x_ticks_count``: The amount of ticks to be shown on the x-axis (not
      required).
    - ``date_format``: Defines the date format as explained
      `here <https://docs.python.org/3/library/datetime.
      html#strftime-and-strptime-format-codes>`_.
    - The keys ``y1``, ``y2`` and ``y3`` are used for the graphs.
      Each graph has to contain the following:

        - ``input``: List which specifies the columns from which to take the
          data.
        - ``factor``: Factor with which the results are multiplied at the end.
        - ``labels``: Labels for each line which will be given in the legend
          (not required) at the end of this section.
          (Defaults to the default-type.)
        - ``min``: The minimum value to which the y-axis will be set.
        - ``max``: The maximum value to which the y-axis will be set.

- ``description``:

    - ``title``: Title which will be displayed on the plot.
    - ``x_axis``: Label of the x-axis
    - ``y_axes``: List with all labels for the y-axes.

- ``graph``:

    - ``height_px``: Specifies the height of the plot in pixels (not required).
    - ``width_px``: Specifies the width of the plot in pixels (not required).
    - ``dpi``: Specifies how many pixels per inch the plot will have (not
      required).
    - ``show``: Specifies whether the plot will be shown directly after
      generating it. (Defaults to False.)
    - ``save``: Specifies whether the plot will be saved. (Defaults to False.)
    - ``format``: Specifies the format in which the plot will be saved.
      (Defaults to `png`.)

.. code-block:: yaml
    :linenos:

    LINE_1:
      name: "cell_voltage_current"
      time:
      mapping:
        x: Test Time
        x_ticks_count: 7
        date_format: "%H:%M:%S"
        y1:
          input:
            - Voltage(V)
          factor: 1
          min: 3.85
          max: 4.3
          labels:
            - Cell Voltage
        y2:
          input:
            - Current(A)
          labels:
            - Current
      description:
        title: Cell Voltage, Current
        x_axis: Date
        y_axes:
          - Cell Voltage (V)
          - Current (A)
      graph:
        height_px: 550
        width_px: 800
        dpi: 100
        show: false
        save: true
        format: png


Example
-------

The following shows how to plot
:download:`this data file <img/example_data.csv>`
using this
:download:`data configuration file <img/csv_config.yaml>`
and this
:download:`plot configuration file <img/plot_config.yaml>`.
