.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _FOX_ETL:

etl
===

The etl command line interface (cli) provides functionalities to preprocess
battery data provided by |foxbms|.
etl is an abbreviation for Extract, Transform and Load, which is a common
approach in the context of data engineering.
With etl a data pipeline can be defined, which extracts various data from
different data sources, transforming them into an uniform data format and loads
those into a database (data warehouse) for further analyzing.
In the current status, the etl cli supports a logfile from a CAN bus as data
source and provides a filter (extract) function to select desired CAN messages
from it.
Subsequently these filtered data can be decoded (transform) and converted to
a structured data format (transform).
The following description is divided into the following sections:

.. contents::
    :depth: 1
    :local:

Preprocessing Concept
---------------------

The goal is to transform the data sent by the CAN bus to a structured data
format (table), containing all information of the system at each point in time
for the later analysis or :ref:`visualization <FOX_PLOT>`.

Typically multiple devices are connected to a CAN bus, where each device tries
to send periodically its messages.
CAN messages contain an ID and one or multiple signals, where each signal has
a name, a value in hexadecimal representation and a physical unit.
A log file of a CAN bus could look like

.. literalinclude:: txt/can_log.txt
    :caption: CAN log file

where the first column is the timestamp, the third column is the CAN ID
and the data of the signals begin at column 7.
For simplification, the CAN log file contains only the system current and a few
cell voltages.
The data in the CAN log file can be seen as irregular time series (varying time
interval between two timestamps) with missing values (no current value is
available at the moment cell voltages were sent) in a semi-structured data
format caused by the serial communication of a CAN bus.

.. _filter:

To avoid unnecessary payload in the later preprocessing steps,
specific CAN messages can be filtered out by etl.
The messages could be filtered by their ID or by their number of occurrence,
so that e.g., only every 10th occurrence of a message remains in the resulting
CAN log file.

.. _decode:

Afterwards the filtered CAN messages can be decoded and sorted based on
their ID into separate files, which will create regular time series without
missing values at each point in time in each file.
The used format for the decoded messages is JSON.
One of those files is depicted below

.. literalinclude:: txt/decoded_can_log.txt
    :caption: Decoded CAN messages

where the first key-value pair is the timestamp and the following
pairs are the signals.
For simplification all decoded CAN messages were shortened.
The key of the signals is a compound of the CAN ID (hex),
``CurrentSensor_SIG_Current`` and the phyiscal unit (|mA|).

.. note::

   By default the physical unit of the timestamps is set to seconds.

.. _table:

To transform the decoded CAN messages to a structured format (table),
the keys are set as column names and the values are used as rows.
At this point the timestamp column is replaced by a date
column in UTC format, where each timestamp is mapped to a date
with respect to start date of the logging.
Each of the aforementioned tables contain a regular time series.
The resulting table for one type of CAN messages is depicted in the table
below.

.. csv-table:: Current
   :file: tables/current.csv
   :widths: 50 50 50
   :header-rows: 1
   :align: center

.. _join:

To obtain one regular time series, all time series could be combined by
a left join.
etl uses as left join method for time series
`join_asof <https://arrow.apache.org/docs/python/generated/pyarrow.Table.html
#pyarrow.Table.join_asof>`_
of `Apache Arrow <https://arrow.apache.org/>`_.
In the context of lithium-ion batteries, most measurements are galvanostatic
and therefore the current as system excitation is a good candidate as left
table in the join.
A table after the join could look like

.. csv-table:: Joined Current and Cell Voltage
   :file: tables/join_table.csv
   :widths: 50 50 50 50
   :header-rows: 1
   :align: center

where all columns are alphanumerical sorted.

.. note::

  By default the method ``join_asof`` is configured to uses previous values
  to fill missing values, therefore the first rows of the joined table will
  contain missing values, because no previous values are available at that
  point of time.

Database & Data Analytics Engine/Libraries
------------------------------------------

Battery data processed as described above are in a structured data format and
therefore in the following we only consider databases and data analytics
engines/libraries handling such structured data.

Considered database management systems (DMS) as well as the data
models of data analytics libraries/engines can be categorized into row or
column oriented.
Most queries in the battery context will read many values
from a few columns.
Hence from a performance perspective, column oriented systems should be
preferred and therfore the etl command supports mostly column
oriented file formats and databases.

DMS store the data and provide usually a SQL interface to query data exceeding
the main memory of the host system.
Known column oriented databases are `DuckDB <https://duckdb.org/>`_ and
`ClickHouse <https://clickhouse.com/>`_ with known row oriented databases as
`MySQL <https://www.mysql.com/>`_ and
`PostgreSQL <https://www.postgresql.org/>`_.
Time series databases as `InfluxDB <https://www.influxdata.com/>`_ are a
special class of DMS providing high performance for time series data with
queries effecting the timestamp/date column.

Data analytics libraries provide no data storage and are limited by the main
memory of the host system.
Most data analytics libraries use an object oriented interface to analyze data
stored in files, which increases the usability of those.
Usually the data model of such libraries is column oriented and known
examples are `Pandas <https://pandas.pydata.org/>`_ and
`Apache Arrow <https://arrow.apache.org/>`_.

To reduce the hardware limitations of data analytics libraries, data analytics
engines were developed which additionally provide a task manager to divide
analytic tasks between computer within a cluster.
`Apache Spark <https://spark.apache.org/>`_ is one of such engines.

At the moment a load functionality to directly upload the data to a
database is not implemented yet.
Hence, the data can only be uploaded into a database via the native file import
of the database or by other tools.
Most data analytics libraries/engines are able to directly query the data
from files, where we recommend to use the supported
`Apache Parquet <https://parquet.apache.org/>`_ file format.

Usage
-----

The etl command is divided into multiple subcommands each providing
specific functionalities described in the previous sections.
Below the general help text of the etl command gives an overview of all
subcommands.

.. include:: ./../../../../build/docs/fox_etl_help.txt

The filter and decode subcommand expect a data stream as input which could
be provided by the command ``cat``.

.. note::

   Piping to standard input does only work when the required Python environment
   is installed.

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         Get-Content -Raw CAN_LOG_FILE | .\fox.ps1 etl decode -c DECODE_CONFIG_FILE -o OUTPUT_DIRECTORY

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         cat CAN_LOG_FILE | ./fox.sh etl decode -c DECODE_CONFIG_FILE \
         -o OUTPUT_FOLDER

   .. group-tab:: Linux

      .. code-block:: shell

         cat CAN_LOG_FILE | ./fox.sh etl decode -c DECODE_CONFIG_FILE \
         -o OUTPUT_FOLDER

Moreover the filter subcommand provides a data stream as output and
therefore it can be used ahead of the decode subcommand.

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         Get-Content -Raw CAN_LOG_FILE | .\fox.ps1 etl filter -c FILTER_CONFIG_FILE | .\fox.ps1 etl decode -c DECODE_CONFIG_FILE -o OUTPUT_DIRECTORY

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         cat CAN_LOG_FILE | ./fox.sh etl filter -c FILTER_CONFIG_FILE \
         | ./fox.sh etl decode -c DECODE_CONFIG_FILE -o OUTPUT_DIRECTORY

   .. group-tab:: Linux

      .. code-block:: shell

         cat CAN_LOG_FILE | ./fox.sh etl filter -c FILTER_CONFIG_FILE \
         | ./fox.sh etl decode -c DECODE_CONFIG_FILE -o OUTPUT_DIRECTORY

More complex data pipelines can be created with
`Apache Airflows <https://airflow.apache.org/>`_ or
`Azure Data Factory
<https://azure.microsoft.com/en-us/products/data-factory>`_.

filter Usage
^^^^^^^^^^^^

The filter subcommand is used to filter out CAN messages from a CAN log file as
described in this :ref:`paragraph <filter>`.
The input and output of the command is a data stream.
The subcommand is executed as described below.

.. include:: ./../../../../build/docs/fox_etl_filter_help.txt

A configuration file of the subcommand could look like

.. literalinclude:: yml/filter.yml
    :language: yaml
    :caption: Configuration for filter subcommand

The key ``id_pos`` defines the position of the CAN IDs in the CAN log file,
``ids`` is a list of all CAN IDs that should be included in the resulting file.
The optional parameter ``sampling`` filters the CAN IDs based on occurrence.
The example configuration file can be downloaded
:download:`here <yml/filter.yml>`.

decode Usage
^^^^^^^^^^^^

The decode subcommand is used to decode CAN messages as described in this
:ref:`paragraph <decode>`.
The input of the command is a data stream.
The subcommand is executed as described below.

.. include:: ./../../../../build/docs/fox_etl_decode_help.txt

A configuration file of the subcommand could look like

.. literalinclude:: yml/decode.yml
    :language: yaml
    :caption: Configuration for decode subcommand

The key ``dbc`` defines the path to the used DBC file, ``timestamp_pos`` is the
column position of the timestamp within the CAN log file, the ``id_pos`` is the
column position of the CAN IDs and ``data_pos`` is the start column position of
the data in each message. The example configuration file can be downloaded
:download:`here <yml/decode.yml>`.

.. note::

  The count for the position starts with 0

table Usage
^^^^^^^^^^^

The table subcommand is used to convert the decoded CAN messages to tables as
described in this :ref:`paragraph <table>`.
Optionally the table subcommand can be used to join these tables to one table
as described in this :ref:`paragraph <join>`.
The subcommand is executed as described below.

.. include:: ./../../../../build/docs/fox_etl_table_help.txt

One to One:
"""""""""""

In case only one file of decoded CAN messages should be converted to a table,
the configuration file for the table subcommand could look like

.. literalinclude:: yml/table_one_one.yml
    :language: yaml
    :caption: Configuration for table subcommand - One to One

with the output parameter as path to a file.
``start_date`` defines the date in UTC format at which the CAN logging has
started.

.. note::

   If an output format is specified in the configuration file, the table
   command converts each `json` file into a separate table as explained below
   in the use-case many to many.

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1 etl table -c table_one_one.yml -o OUTPUT_FILE.csv INPUT_FILE.csv

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         ./fox.sh etl table -c table_one_one.yml -o OUTPUT_FILE.csv INPUT_FILE.csv

   .. group-tab:: Linux

      .. code-block:: shell

         ./fox.sh etl table -c table_one_one.yml -o OUTPUT_FILE.csv INPUT_FILE.csv

Many to One:
""""""""""""

If multiple files with decoded CAN messages should be converted and joined to
one table, the configuration file for the table subcommand could look like

.. literalinclude:: yml/table_many_one.yml
    :language: yaml
    :caption: Configuration for table subcommand - Many to One

with ``join_on`` defining the column of the left table in the join.
The output parameter must be the path to a file.

.. note::

   If an output format is specified in the configuration file, the table
   command converts each `json` file into a separate table as explained below
   in the use-case many to many.

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1 etl table -c table_many_one.yml -o OUTPUT_FILE.csv INPUT_FOLDER

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         .\fox.sh etl table -c table_many_one.yml -o OUTPUT_FILE.csv INPUT_FOLDER

   .. group-tab:: Linux

      .. code-block:: shell

         .\fox.sh etl table -c table_many_one.yml -o OUTPUT_FILE.csv INPUT_FOLDER

Many to Many:
"""""""""""""

In case each file with decoded CAN message should be converted to a table,
without any join, the configuration file for the table command could look like

.. literalinclude:: yml/table_many_many.yml
    :language: yaml
    :caption: Configuration for table subcommand - Many to Many

with ``output_format`` as ``csv`` or ``parquet`` defining the file format
at which all tables are saved.
The output parameter must be the path to a folder.

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1 etl table -c table_many_many.yml -o OUTPUT_FOLDER INPUT_FOLDER

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         .\fox.sh etl table -c table_many_many.yml -o OUTPUT_FOLDER INPUT_FOLDER

   .. group-tab:: Linux

      .. code-block:: shell

         .\fox.sh etl table -c table_many_many.yml -o OUTPUT_FOLDER INPUT_FOLDER

.. note::

   One file with decoded CAN message can not be converted to multiple tables!

If the timestamp values of a CAN log are not in seconds, the table subcommand
is able to correctly convert these values to the needed phyiscal unit with the
optional parameter ``timestamp_factor``.
Internally all timestamp values are multiplied with the ``timestamp_factor``
to interpred these values as duration in microseconds.
The default value of ``timestamp_factor`` is 1000000 for timestamp values in
seconds.
If the timestamp values are in milliseconds, the ``timestamp_factor`` needs
to be 1000.

The aforementioned
`join_asof <https://arrow.apache.org/docs/python/generated/pyarrow.Table.html
#pyarrow.Table.join_asof>`_ defines with a tolerance parameter how to fill
missing values with previous values (forward fill).
By default the tolerance value is set to -100000, where the minus indicates a
forward fill and the 100000 indicates the maximum considered time difference.
This tolerance parameter can be changed by the optional parameter ``tolerance``
in the configuration file.

All example configuration files for the table subcommand can be download below:

| :download:`One to One <yml/table_one_one.yml>`
| :download:`Many to One <yml/table_many_one.yml>`
| :download:`Many to Many <yml/table_many_many.yml>`

Build pip Package
-----------------

etl can be packaged to a standalone pip package. First change your working
directory to the root of the repository, if this is not already the case.
Afterwards the pip package can be build with

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1 run-program python -m build cli\cmd_etl\ -o .\dist

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         ./fox.sh run-program python -m build cli/cmd_etl/ -o ./dist

   .. group-tab:: Linux

      .. code-block:: shell

         ./fox.sh run-program python -m build cli/cmd_etl/ -o ./dist

where the resulting WHEEL file can be found in the folder
``ROOT_REPOSITORY/dist``.

The pip package can be installed into the active environment with

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         python -m pip install .\dist\WHEEL_NAME

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         python -m pip install ./dist/WHEEL_NAME

   .. group-tab:: Linux

      .. code-block:: shell

         python -m pip install ./dist/WHEEL_NAME

Now all etl subcommands can be executed with ``foxetl SUBCOMMAND``.
