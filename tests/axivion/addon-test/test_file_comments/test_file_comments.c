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
// include: ltc_defs.h
// include: not_doxygen_comment.c
// include: ok.c
// include: ok.h
// include: text_error.c
// include: unexpected_tag.c
// include: value_error.c
// include: interlock.c
// setup: cafeCC(args=[TESTCASE.filename, 'not_doxygen_comment.c', 'ok.c', 'text_error.c', 'unexpected_tag.c', 'value_error.c', 'interlock.c'])
// test: analysis(['IISB-DoxygenFileComment'])

#include "ltc_defs.h"

// test: TESTCASE.add_expected_stdout(f"test_file_comments.c:13:1: error: Expect a Doxygen comment here (Rule IISB-DoxygenFileComment)")

int main()
{
}
