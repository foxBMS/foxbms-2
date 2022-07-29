// primary_file: test_doxygen_comments.cpp

#include "header.h"

class X
//^stdout:7: error: Need Doxygen comment for this entity. [X] (Rule IISB-DoxygenCommentAtDeclaration)
{
};

void only_defined_func()
//^stdout:6: error: Need Doxygen comment for this entity. [only_defined_func()] (Rule IISB-DoxygenCommentAtDeclaration)
{
}

void in_header_declared_func()
{
}

void in_cpp_declared_func();
//^stdout:6: error: Need Doxygen comment for any declaration of this entity. [in_cpp_declared_func()] (Rule IISB-DoxygenCommentAtDeclaration)

void in_cpp_declared_func()
{
}


void mixed_declared_func_nok();  // label:mixed_declared_func_nok__decl_in_cpp
//^stdout:6: error: Need Doxygen comment for any declaration of this entity. [mixed_declared_func_nok()] (Rule IISB-DoxygenCommentAtDeclaration), $(mixed_declared_func_nok__decl_in_h):6:

void mixed_declared_func_nok()
{
}


void mixed_declared_func_ok();

void mixed_declared_func_ok()
{
}

static T1 var_1 = 0;
//^stdout:11: error: Need Doxygen comment for this entity. [var_1] (Rule IISB-DoxygenCommentAtDeclaration)

/** Hallo */
static T2 var_2 = 0;
