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

"""Testing file 'cli/cmd_ci/check_ci_config.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path

try:
    from cli.cmd_ci import check_ci_config
    from cli.cmd_ci.check_ci_config import Stage
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_ci import check_ci_config
    from cli.cmd_ci.check_ci_config import Stage


class TestCheckCiConfig(unittest.TestCase):
    """Test of the function all_software_available"""

    @classmethod
    def setUpClass(cls):
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem
        cls.CI_MAIN_CONFIG = check_ci_config.CI_MAIN_CONFIG
        cls.CI_PIPELINE_DIR = check_ci_config.CI_PIPELINE_DIR

    def tearDown(self):
        check_ci_config.CI_MAIN_CONFIG = self.CI_MAIN_CONFIG
        check_ci_config.CI_PIPELINE_DIR = self.CI_PIPELINE_DIR

    def test_read_config_0(self):
        "Test function 'read_config', test 0."
        check_ci_config.CI_MAIN_CONFIG = Path(__file__)
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            check_ci_config.read_config()
        self.assertRegex(
            cm.exception.code,
            r".*test_check_ci_config.py is not a valid yaml file\.",
        )

    def test_read_config_invalid_yml_file(self):
        "Test function 'read_config': empty yml file passed"
        check_ci_config.CI_MAIN_CONFIG = self.tests_dir / "empty.yml"
        with self.assertRaises(SystemExit) as cm:
            check_ci_config.read_config()
        err = f"'{check_ci_config.CI_MAIN_CONFIG}' is empty."
        self.assertEqual(cm.exception.code, err)

    def test_read_config_no_include(self):
        "Test function 'read_config': yml file does not contain 'include' key."
        check_ci_config.CI_MAIN_CONFIG = self.tests_dir / "no_include.yml"
        with self.assertRaises(SystemExit) as cm:
            check_ci_config.read_config()
        err = f"Expected 'include' key to be defined in '{check_ci_config.CI_MAIN_CONFIG}'."
        self.assertEqual(cm.exception.code, err)

    def test_read_config_invalid_include(self):
        "Test function 'read_config', test 3."
        check_ci_config.CI_MAIN_CONFIG = self.tests_dir / "invalid_include.yml"
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            check_ci_config.read_config()
        # see the test file for details how this test works
        self.assertEqual(
            cm.exception.code,
            "Could not find some included file(s) or files are not valid yml files.",
        )
        self.assertRegex(
            buf.getvalue(),
            r".*test_check_ci_config\.py' is not a valid yml file\.\n"
            r"Could not find file 'foo\.yml'\.\n",
        )

    def test_read_config_4(self):
        "Test function 'read_config', test 4."
        check_ci_config.CI_MAIN_CONFIG = self.tests_dir / "valid.yml"
        self.assertEqual(
            check_ci_config.read_config(),
            {"include": [{"local": "tests/cli/cmd_ci/test_check_ci_config/valid.yml"}]},
        )

    def test_get_stages(self):
        """Test 'get_stages' function"""
        # no stages defined
        with self.assertRaises(SystemExit) as cm:
            check_ci_config.get_stages({})
        self.assertEqual(cm.exception.code, "Could not determine stages.")
        stages = check_ci_config.get_stages({"stages": ["some_test"]})
        self.assertEqual(stages, [check_ci_config.Stage(name="some_test", prefix="st")])

    def test_check_ci_config(self):
        """Test 'check_ci_config' function: test that the CI file exists."""
        # Test 1: Fail
        check_ci_config.CI_MAIN_CONFIG = Path("foo")
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.check_ci_config()
        self.assertEqual(buf.getvalue(), "File 'foo' does not exist.\n")
        self.assertEqual(result, 1)
        # Test 2: Test success is covered by a valid input file later

    def test_match_stages_with_files(self):
        """Test 'match_stages_with_files' function: test that the number of
        defined stages matches the number if included files via key
        'include:↳local'.
        """
        # Test 1: Fail
        ci_config = {
            "include": [{"local": "foo"}, {"local": "bar"}, {"local": "dummy"}]
        }
        stages = ["blu"]
        stage_files = [Path("blu")]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.match_stages_with_files(
                ci_config, stages, stage_files
            )
        self.assertEqual(result, 1)
        self.assertRegex(
            buf.getvalue(),
            r"Number of included stages \(3\) does not match the number stage files \(1\)\.\n"
            r"included files\: bar\ndummy\nfoo\nstage files: blu\n",
        )
        # Test 2: Fail
        ci_config = {
            "include": [{"local": "foo"}, {"local": "bar"}, {"local": "dummy"}]
        }
        stages = ["stage_1", "stage_2"]
        # any valid yml file can be used for this test
        stage_files = [Path("tests/cli/cmd_ci/test_check_ci_config/valid.yml")]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.match_stages_with_files(
                ci_config, stages, stage_files
            )
        self.assertEqual(result, 1)
        self.assertRegex(
            buf.getvalue(),
            r"Number of stages \(2\) does not match the number stage files "
            r"\(1\) found in '.*ci_pipeline'\.",
        )

        # Test 3: Success
        stage = Path("tests/cli/cmd_ci/test_check_ci_config/valid.yml")
        ci_config = {"include": [{"local": stage}]}
        stages = ["valid"]
        stage_files = [stage]
        result = check_ci_config.match_stages_with_files(ci_config, stages, stage_files)
        self.assertEqual(result, 0)

    def test_check_job_prefix(self):
        """Test 'check_job_prefix' function, test that the job prefix matches."""
        # Test 1: Fail
        ci_config = {"fa_job0": ""}
        stages = [Stage("stage_1", "s1"), Stage("stage_2", "s2")]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.check_job_prefix(ci_config, stages)
        self.assertEqual(result, 1)
        self.assertEqual(buf.getvalue(), "Job 'fa_job0' uses the wrong prefix.\n")

        # Test 2: Success
        ci_config = {"s1_job0": ""}
        stages = [Stage("stage_1", "s1"), Stage("stage_2", "s2")]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.check_job_prefix(ci_config, stages)
        self.assertEqual(result, 0)

    def test_check_emb_spa_build_alignment(self):
        """Test 'check_emb_spa_build_alignment' function"""
        # Test 1: Fail
        ci_stages = {}
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.check_emb_spa_build_alignment(ci_stages)
        self.assertEqual(result, 1)
        self.assertEqual(
            buf.getvalue(),
            "key '.parallel-build_app_embedded-template' is not defined.\n",
        )

        # Test 2: Fail
        ci_stages = {".parallel-build_app_embedded-template": 1}
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.check_emb_spa_build_alignment(ci_stages)
        self.assertEqual(result, 1)
        self.assertEqual(
            buf.getvalue(),
            "key '.parallel-build_app_spa-template' is not defined.\n",
        )

        # Test 3: Fail
        ci_stages = {
            ".parallel-build_app_embedded-template": 1,
            ".parallel-build_app_spa-template": 2,
        }
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.check_emb_spa_build_alignment(ci_stages)
        self.assertEqual(result, 1)
        self.assertRegex(
            buf.getvalue(),
            r"""Error:The build matrix defined in
  .*build_app_embedded\.yml\: key '\.parallel-build_app_embedded-template'
and
  .*static_program_analysis\.yml\: key '\.parallel-build_app_spa-template'
need to be equal\.""",
        )

        # Test 4: Success
        ci_stages = {
            ".parallel-build_app_embedded-template": 1,
            ".parallel-build_app_spa-template": 1,
        }

        result = check_ci_config.check_emb_spa_build_alignment(ci_stages)
        self.assertEqual(result, 0)

    def test_validate_build_app_matrix(self):
        """Test 'validate_build_app_matrix' function"""
        # Test 1: Fail
        ci_config = {}
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.validate_build_app_matrix(ci_config)
        self.assertEqual(result, 1)
        self.assertEqual(
            buf.getvalue(),
            "Key '.parallel-build_app_embedded-template:↳parallel:↳matrix' "
            "is missing or uses a wrong format.\n",
        )

        # Test 2: Success
        ci_config = {
            ".parallel-build_app_embedded-template": {
                "parallel": {
                    "matrix": 1,
                }
            }
        }
        result = check_ci_config.validate_build_app_matrix(ci_config)
        self.assertEqual(result, 0)

    def test_get_expected_artifacts(self):
        """Test 'get_expected_artifacts' function"""
        ci_config = {
            ".parallel-build_app_embedded-template": {
                "parallel": {
                    "matrix": [
                        {"a": 1, "b": 2},
                        {"a": 3, "b": 4},
                        {"a": 5, "b": 6},
                    ],
                }
            }
        }
        expected_artifacts = [
            "bae_all_config_variants: [1, 2]",
            "spa_build: [1, 2]",
            "bae_all_config_variants: [3, 4]",
            "spa_build: [3, 4]",
            "bae_all_config_variants: [5, 6]",
            "spa_build: [5, 6]",
        ]
        artifacts = check_ci_config.get_expected_artifacts(ci_config)
        self.assertEqual(artifacts, expected_artifacts)

    def test_validate_spae_artifacts(self):
        """Test function 'validate_spae_artifacts'"""
        # Test 1: Fail
        ci_config = {}
        dummy = []
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.validate_spae_artifacts(ci_config, dummy)
        self.assertEqual(result, 1)
        self.assertEqual(
            buf.getvalue(),
            "Key 'spae_gather_spa_artifacts:↳needs' is missing or uses a wrong "
            "format.\n",
        )

        # Test 1: Success
        ci_config = {
            ".parallel-build_app_embedded-template": {
                "parallel": {
                    "matrix": [
                        {"a": 1, "b": 2},
                        {"a": 3, "b": 4},
                        {"a": 5, "b": 6},  # these will be missing in the test
                    ],
                }
            },
            "spae_gather_spa_artifacts": {
                "needs": [
                    "bae_all_config_variants: [1, 2]",
                    "spa_build: [1, 2]",
                    "bae_all_config_variants: [3, 4]",
                    "spa_build: [3, 4]",
                ]
            },
        }
        expected = check_ci_config.get_expected_artifacts(ci_config)
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.validate_spae_artifacts(ci_config, expected)
        self.assertEqual(result, 1)
        self.assertEqual(
            buf.getvalue(),
            "Some artifacts are missing in job 'spae_gather_spa_artifacts':\n"
            "Expected: ['bae_all_config_variants: [1, 2]', "
            "'bae_all_config_variants: [3, 4]', "
            "'bae_all_config_variants: [5, 6]', "
            "'spa_build: [1, 2]', 'spa_build: [3, 4]', 'spa_build: [5, 6]']\n"
            "Found: ['bae_all_config_variants: [1, 2]', "
            "'bae_all_config_variants: [3, 4]', 'spa_build: [1, 2]', "
            "'spa_build: [3, 4]']\n",
        )

    def test_validate_hil_test_config(self):
        """Test 'validate_hil_test_config' function"""
        # Test 1: Fail
        ci_config = {
            "th_ensure_power_supply_is_off": "dummy",
            "th_some_test": "dummy",
            "th_switch_power_supply_off": {
                "needs": [
                    "th_ensure_power_supply_is_off",
                ],
            },
        }
        dummy = [""]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.validate_hil_test_config(ci_config, dummy)
        self.assertEqual(result, 2)
        self.assertEqual(
            "th_some_test does not specify any artifacts.\n"
            "Key 'th_switch_power_supply_off:↳needs' is missing test jobs.\n",
            buf.getvalue(),
        )
        # Test 2: Fail (needs is not correct for basic jobs)
        ci_config = {
            "th_ensure_power_supply_is_off": "dummy",
            "th_flash_and_test_bootloader": {
                "needs": [
                    "th_ensure_power_supply_is_off",
                    "bbe_tms570",
                ],
            },
            "th_some_test": {
                "needs": [
                    "foo",
                ]
            },
            "th_switch_power_supply_off": {
                "needs": [
                    "th_ensure_power_supply_is_off",
                    "th_flash_and_test_bootloader",
                    "th_some_test",
                ]
            },
        }

        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.validate_hil_test_config(ci_config, dummy)
        self.assertEqual(result, 4)
        self.assertEqual(
            "The second last entry for 'th_flash_and_test_bootloader:↳needs:' "
            "shall be 'th_bootload_on_hardware_unit_tests'.\n"
            "The first entry for 'th_some_test:↳needs:' shall be "
            "'th_ensure_power_supply_is_off'.\nThe last entry for "
            "'th_some_test:needs:' shall be 'th_flash_and_test_bootloader'.\n"
            "Too few artifacts specified in 'th_some_test:needs:'.\n",
            buf.getvalue(),
        )

        # Test 3: Fail (needs is not correct for basic jobs)
        ci_config = {
            "th_ensure_power_supply_is_off": "dummy",
            "th_flash_and_test_bootloader": {
                "needs": [
                    "th_ensure_power_supply_is_off",
                    "bbe_tms570",
                ]
            },
            "th_some_test": {
                "needs": [
                    "foo",
                    "foo1",
                    "foo2",
                    "foo3",
                ]
            },
            "th_switch_power_supply_off": {
                "needs": [
                    "th_ensure_power_supply_is_off",
                    "th_flash_and_test_bootloader",
                    "th_some_test",
                ]
            },
        }
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.validate_hil_test_config(ci_config, dummy)
        self.assertEqual(result, 5)
        self.assertEqual(
            "The second last entry for 'th_flash_and_test_bootloader:↳needs:' "
            "shall be 'th_bootload_on_hardware_unit_tests'.\n"
            "The first entry for 'th_some_test:↳needs:' shall be "
            "'th_ensure_power_supply_is_off'.\nThe last entry for "
            "'th_some_test:needs:' shall be 'th_flash_and_test_bootloader'.\n"
            "Too many artifacts specified in 'th_some_test:needs:'.\n"
            "The test 'th_some_test' specifies an artifact that is is not "
            "specified in '.parallel-build_app_embedded-template'.\n",
            buf.getvalue(),
        )

        # Test 4: Fail (needs is not correct for basic jobs)
        ci_config = {
            "th_ensure_power_supply_is_off": "dummy",
            "th_bootload_on_hardware_unit_tests": {
                "needs": ["th_ensure_power_supply_is_off", "foo"],
            },
            "th_flash_and_test_bootloader": {
                "needs": ["th_ensure_power_supply_is_off", "bbe_tms570", "foo"],
            },
            "th_some_test": {
                "needs": [
                    "th_ensure_power_supply_is_off",
                    "bae_all_config_variants: [a0, b0, c0]",
                    "th_flash_and_test_bootloader",
                ]
            },
            "th_switch_power_supply_off": {
                "needs": [
                    "th_ensure_power_supply_is_off",
                    "th_flash_and_test_bootloader",
                    "th_some_test",
                ]
            },
        }
        expected_artifacts = ["bae_all_config_variants: [a0, b0, c0]"]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.validate_hil_test_config(
                ci_config, expected_artifacts
            )
        self.assertEqual(result, 4)
        self.assertEqual(
            "The last entry for 'th_bootload_on_hardware_unit_tests:↳needs:' "
            "shall be 'bbe_tms570_on_hardware_unit_test'.\n"
            "The last entry for 'th_flash_and_test_bootloader:↳needs:' shall "
            "be 'bbe_tms570'.\n"
            "The second last entry for 'th_flash_and_test_bootloader:↳needs:' "
            "shall be 'th_bootload_on_hardware_unit_tests'.\n"
            "Key 'th_switch_power_supply_off:↳needs' is missing test jobs.\n",
            buf.getvalue(),
        )

    def test_analyze_config(self):
        """Test 'analyze_config' function."""
        # Test 1: Fail
        ci_config = {
            "stages": ["configure", "dummy"],
        }
        stage_files = []
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.analyze_config(ci_config, stage_files)
        self.assertEqual(result, 1)
        self.assertRegex(
            buf.getvalue(),
            r"Number of stages \(1\) does not match the number stage files "
            r"\(0\) found in '.*ci_pipeline'\.",
        )

        # Test 2: Fail
        ci_config["fa_job0"] = "bla"
        ci_config["include"] = (
            [
                {"local": "tests/cli/cmd_ci/test_check_ci_config/valid.yml"},
            ],
        )

        stage_files = [Path("tests/cli/cmd_ci/test_check_ci_config/valid.yml")]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.analyze_config(ci_config, stage_files)
        self.assertEqual(result, 1)
        self.assertEqual(buf.getvalue(), "Job 'fa_job0' uses the wrong prefix.\n")

        # Test 3: Fail
        del ci_config["fa_job0"]
        ci_config["d_job0"] = "bla"

        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.analyze_config(ci_config, stage_files)
        self.assertEqual(result, 1)
        self.assertEqual(
            buf.getvalue(),
            "key '.parallel-build_app_embedded-template' is not defined.\n",
        )

        # Test 4: Fail
        ci_config[".parallel-build_app_embedded-template"] = "foo"
        ci_config[".parallel-build_app_spa-template"] = "foo"

        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.analyze_config(ci_config, stage_files)
        self.assertEqual(result, 1)
        self.assertEqual(
            buf.getvalue(),
            "Key '.parallel-build_app_embedded-template:↳parallel:↳matrix' "
            "is missing or uses a wrong format.\n",
        )

        # Test 5: Fail
        ci_config[".parallel-build_app_embedded-template"] = {
            "parallel": {
                "matrix": [
                    {"a": 1, "b": 2},
                    {"a": 3, "b": 4},
                    {"a": 5, "b": 6},
                ],
            }
        }
        ci_config[".parallel-build_app_spa-template"] = {
            "parallel": {
                "matrix": [
                    {"a": 1, "b": 2},
                    {"a": 3, "b": 4},
                    {"a": 5, "b": 6},
                ],
            }
        }
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_ci_config.analyze_config(ci_config, stage_files)
        self.assertEqual(result, 1)
        self.assertEqual(
            buf.getvalue(),
            "Key 'spae_gather_spa_artifacts:↳needs' is missing or uses a wrong format.\n",
        )

    def test_check_ci_config_good(self):
        """Test 'check_ci_config' function, when the configuration is valid."""
        check_ci_config.CI_MAIN_CONFIG = self.tests_dir / "good.yml"
        check_ci_config.CI_PIPELINE_DIR = self.tests_dir / "good"
        result = check_ci_config.check_ci_config()
        self.assertEqual(result, 0)


if __name__ == "__main__":
    unittest.main()
