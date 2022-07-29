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
// include: a.c
// include: a.cpp
// include: a.h
// include: sub/a.c
// setup: cafeCC(args=[TESTCASE.filename, 'a.c', 'a.cpp', 'sub/a.c'])
// test: analysis(['IISB-UniqueFileNameCheck'])

// test: TESTCASE.add_expected_stdout("a.c:1:1: error: File names are too similar [a.c] (Rule IISB-UniqueFileNameCheck), sub/a.c:1:1:")
// test: TESTCASE.add_expected_stdout("sub/a.c:1:1: error: File names are too similar [sub/a.c] (Rule IISB-UniqueFileNameCheck), a.c:1:1:")


#include "a.h"

int main()
{
    first();
    second();
    third();
}
