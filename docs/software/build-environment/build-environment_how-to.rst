.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _CHANGING_AND_EXTENDING_THE_BUILD_ENVIRONMENT:

Changing and Extending the Build Environment
============================================

If packages are needed that are not included in development environment they can simply be added. This how-to explains
it for Windows. If there is a reference to |conda_env_config_win32| and you are on Linux replace it by
|conda_env_config_linux|.

#. Cloning the current environment

   #. Get the name of the current development environment from |conda_env_config_win32|.

      .. literalinclude:: ./../../../conf/env/conda_env_win32.yaml
         :language: yaml
         :emphasize-lines: 1
         :lines: 1-7,126-131
         :caption: Shortened snippet from |conda_env_config_win32| that shows the development environment name.

   #. Clone the current development environment by activating the base environment and using the following command:

      .. code-block:: console

         C:\Users\vulpes>%USERPROFILE%\miniconda3\Scripts\activate base
         (base) C:\Users\vulpes>conda create --name NEW_ENV_NAME --clone CURRENT_ENV_NAME

#. Activate the new development environment:

      .. code-block:: console

         (base) C:\Users\vulpes>conda activate NEW_ENV_NAME

#. Optional: Update all conda packages:

      .. code-block:: console

        (NEW_ENV_NAME) C:\Users\vulpes>conda update --all

#. Optional: Update pip packages. It is not recommended to bulk update all pip
   packages. It is best to get a list of all pip dependencies

      .. code-block:: console

        (NEW_ENV_NAME) C:\Users\vulpes>conda list | findstr /I "pypi"
        markdown                  2.6.11                   pypi_0    pypi
        ...

   and then update these one by one and check for errors and role back in case
   of errors:

      .. code-block:: console

        (NEW_ENV_NAME) C:\Users\vulpes>pip install markdown --upgrade

#. Install/Update/Remove the packages accordingly to your needs using ``conda`` or ``pip``:

      .. code-block:: console

        (NEW_ENV_NAME) C:\Users\vulpes>conda install my_package
        (NEW_ENV_NAME) C:\Users\vulpes>pip install my_other_package

#. Export the new development environment:

      .. code-block:: console

        (NEW_ENV_NAME) C:\Users\vulpes>conda env export > path\to\conf\env\conda_env_win32.yaml

#. Remove the ``Prefix`` entry from |conda_env_config_win32|.
#. Commit the new environment to the repository.


Further Reading
---------------

An explanation why build environments are used is found in :ref:`BUILD_ENVIRONMENT`.
