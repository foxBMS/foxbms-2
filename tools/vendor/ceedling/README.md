# Ceedling

This directory contains the vendored files of Ceedling, the unit-test-runner
used by this project.
In order to use it, you have to install all necessary dependencies by calling
`bundle install` **in this directory**.

## Using Ceedling Directly

Ceedling can be used outside of the implemened toolchain by using the following
steps:

- Make the correct Ruby version available in PATH (see
  `<root>/conf/env/paths_win32.txt`)
- Copy `<root>/conf/unit/app_project_win32.yml` or
  `<root>/conf/unit/app_project_posix.yml` to
  `<root>/build/app_host_unit_test/project.yml`
- Generate the HAL sources from the HALCoGen files for the project
  (`conf/hcg/app.hcg`)
- Copy the HALCoGen generated directories (`source` and `include`) to
  `<root>/build/app_host_unit_test/`.
- `cd` into `<root>/build/app_host_unit_test/` and run Ceedling commands
  (e.g., `ceedling test:test_adc.c`)

## How to Update the Vendored Ceedling

- Check the Ceedling license of the latest release on
  <https://github.com/ThrowTheSwitch/Ceedling>.
  If it is valid to be used in the foxBMS project and therefore generally to be
  updated.
  If it is needed, update the license information of `license.txt` in this
  directory.
- Download the desired Ceedling gem from
  <https://github.com/ThrowTheSwitch/Ceedling/releases>.
- Download the `Gemfile` and `Gemfile.lock` for the specific release from
  <https://github.com/ThrowTheSwitch/Ceedling/tags>.
- Make the correct Ruby version available in PATH (see
  `<root>/conf/env/paths_win32.txt`)
- Install the latest Ceedling version, e.g., by

  ```bash
  # replace file name of the gem with the actually downloaded one
  gem install path/to/ceedling-x.y.z-abcdef.gem
  ```

- Run in repository root

  ```bash
  # assuming we are in the repository root
  # create the files to be vendored
  ceedling new --local tmp
  # if on Windows, convert the files to CRLF
  pushd tmp/vendor/ceedling
  find . -type f -print0 | xargs -0 unix2dos
  popd
  # remove the old vendored files
  rm -rf tools/vendor/ceedling/bin
  rm -rf tools/vendor/ceedling/lib
  rm -rf tools/vendor/ceedling/plugins
  rm -rf tools/vendor/ceedling/vendor
  rm -f tools/vendor/ceedling/GIT_COMMIT_SHA
  rm -f tools/vendor/ceedling/license.txt
  # copy the new Ceedling version
  cp -r tmp/vendor/ceedling/bin/ tools/vendor/ceedling/bin
  cp -r tmp/vendor/ceedling/lib/ tools/vendor/ceedling/lib
  cp -r tmp/vendor/ceedling/plugins/ tools/vendor/ceedling/plugins
  cp -r tmp/vendor/ceedling/vendor/ tools/vendor/ceedling/vendor
  cp tmp/vendor/ceedling/GIT_COMMIT_SHA tools/vendor/ceedling/GIT_COMMIT_SHA
  cp tmp/vendor/ceedling/license.txt tools/vendor/ceedling/license.txt
  ```

- Add and commit the changes.
- Add a changelog entry.
