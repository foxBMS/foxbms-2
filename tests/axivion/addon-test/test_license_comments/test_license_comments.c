/* axivion_config.json:
{
    "Analysis": {
        "Analysis-GlobalOptions": {
            "_additional_rules": [
                "$(TEST_SOURCE_DIR)/../../addon"
            ]
        }
    },
    "_Format": "1.0"
}
*/
// include: ok.c
// include: no_license_comment.h
// include: license_incorrect.c
// include: license_short.c
// setup: cafeCC(args=[TESTCASE.filename, 'ok.c', 'license_incorrect.c', 'license_short.c'])
// test: analysis(['IISB-FileLicenseComment'])

// test: TESTCASE.add_expected_stdout(f"test_license_comments.c:1:1: error: License header is incorrect (Rule IISB-FileLicenseComment)")

#include "no_license_comment.h"
// test: TESTCASE.add_expected_stdout("no_license_comment.h:1:1: error: No license header found (Rule IISB-FileLicenseComment)")


void no_license_comment()
{
}

int main()
{
}
