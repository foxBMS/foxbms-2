/* axivion_config.json:
{
    "Analysis": {
        "Analysis-GlobalOptions": {
            "_additional_rules": [
                "$(TEST_SOURCE_DIR)/../../addon"
            ]
        },
        "IISB-FileEncodingCheck": {
            "alternative_encoding": {
                "$(delta:+)": {
                    "latin1.cpp": "latin_1"
                },
                "$(delta:-)": []
            }
        }
    },
    "_Format": "1.0"
}
*/
// include: utf8.cpp
// include: latin1.cpp
// include: windows-1252.cpp
// setup: cafeCC(args=[TESTCASE.filename, 'utf8.cpp', 'latin1.cpp', 'windows-1252.cpp'])
// test: analysis(['IISB-FileEncodingCheck'])

// test: TESTCASE.add_expected_stdout("windows-1252.cpp:1:1: error: File does not have required encoding utf_8 [windows-1252.cpp] (Rule IISB-FileEncodingCheck)")


int main()
{
}
