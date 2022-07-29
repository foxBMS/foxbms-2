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
// include: file.cpp
// include: header.h
// setup: cafeCC(args=[TESTCASE.filename, 'file.cpp'])
// test: analysis(['IISB-DoxygenCommentAtDeclaration'])


int main()
//^stdout:5: error: Need Doxygen comment for this entity. [main()] (Rule IISB-DoxygenCommentAtDeclaration)
{
}
