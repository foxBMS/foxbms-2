/* axivion_config.json:
{
    "Analysis": {
        "Analysis-GlobalOptions": {
            "_additional_rules": [
                "$(TEST_SOURCE_DIR)/../addon"
            ]
        },
        "IISB-LiteralSuffixesCheck": {
            "consider_int_sign_conversion": true,
            "exclude_in_macros": [
                "__LINE__"
            ]
        }
    },
    "_Format": "1.0"
}
*/
// test: analysis(['IISB-LiteralSuffixesCheck'])

unsigned int first = 4000;
//^ stdout:22: error: Integer literal constant of size 32 requires "u" suffix [4000] (Rule IISB-LiteralSuffixesCheck)
unsigned int second = 0x8000; /* not compliant */
//^ stdout:23: error: Integer literal constant of size 32 requires "u" suffix [0x8000] (Rule IISB-LiteralSuffixesCheck)
unsigned int good1 = 4000U;
//^stdout:22: error: Literal constant suffix has wrong casing, please use "u" [4000U] (Rule IISB-LiteralSuffixesCheck)
unsigned int good2 = 0x8000u;
unsigned int badhex = 0x888a; /* not compliant */
//^ stdout:23: error: Integer literal constant of size 32 requires "u" suffix [0x888a] (Rule IISB-LiteralSuffixesCheck)
unsigned int goodhex = 0x888au;
unsigned int goodlongsuffix = 0ul;
//^stdout:31: error: Integer literal constant of size 32 requires "u" suffix [0ul] (Rule IISB-LiteralSuffixesCheck)
unsigned int alsogood = 0lu;
//^stdout:25: error: Integer literal constant of size 32 requires "u" suffix [0lu] (Rule IISB-LiteralSuffixesCheck)
unsigned int suppress = __LINE__;

float f_first = 0.123;
//^stdout:17: error: Floating point literal constant of size 32 requires "f" suffix [0.123] (Rule IISB-LiteralSuffixesCheck)
float f_second = 0.123E10;
//^stdout:18: error: Floating point literal constant of size 32 requires "f" suffix [0.123E10] (Rule IISB-LiteralSuffixesCheck)
float f_third = 0.23E10F;
//^stdout:17: error: Literal constant suffix has wrong casing, please use "f" [0.23E10F] (Rule IISB-LiteralSuffixesCheck)
float f_fourth = 0xa0.12b3p10;
//^stdout:18: error: Floating point literal constant of size 32 requires "f" suffix [0xa0.12b3p10] (Rule IISB-LiteralSuffixesCheck)
float f_fifth = 0x2e3p10;
//^stdout:17: error: Floating point literal constant of size 32 requires "f" suffix [0x2e3p10] (Rule IISB-LiteralSuffixesCheck)
float f_sixth = 0x2.3p23F;
//^stdout:17: error: Literal constant suffix has wrong casing, please use "f" [0x2.3p23F] (Rule IISB-LiteralSuffixesCheck)

double d_ok_first = 0.123;
double d_second = 0.123E10f;
//^stdout:19: error: Literal constant should have no suffix [0.123E10f] (Rule IISB-LiteralSuffixesCheck)
double d_third = 0.23E10F;
//^stdout:18: error: Literal constant should have no suffix [0.23E10F] (Rule IISB-LiteralSuffixesCheck)
double d_fourth = 0xa0.12p3f;
//^stdout:19: error: Literal constant should have no suffix [0xa0.12p3f] (Rule IISB-LiteralSuffixesCheck)
double d_ok_fifth = 0x2p10;
double d_sixth = 0x2.3p23F;
//^stdout:18: error: Literal constant should have no suffix [0x2.3p23F] (Rule IISB-LiteralSuffixesCheck)

double d_ok_second = 0.123E10;
double d_ok_third = 0.23E10;
double d_ok_fourth = 0xa0.12p3;
double d_ok_sixth = 0x2.3p23;

float f_ok_first = 0.123f;
float f_ok_second = 0.123E10f;
float f_ok_third = 0.23E10f;
float f_ok_fourth = 0xa0.12p3f;
float f_ok_fifth = 0x2p10f;
float f_ok_sixth = 0x2.3p23f;


#if 0  /* ok */
#endif

struct Iterator
{
    Iterator& operator++(const int dummy);
};

void f()
{
    for (Iterator i; true; i++) {} /* avoid message */
    int small = -1;
    if (small < 0) {}  /* ok, not an unsigned context */
}

int main()
{
}
