// primary_file: test_doxygen_comments.cpp

extern void do_something();  // this has no definition
//^stdout:13: error: Need Doxygen comment for any declaration of this entity. [do_something()] (Rule IISB-DoxygenCommentAtDeclaration)

template <class T>
class Dummy
//^stdout:7: error: Need Doxygen comment for this entity. [Dummy] (Rule IISB-DoxygenCommentAtDeclaration)
{
public:
    /** This is a valid comment
    */
    void print();

    void another_print();
    //^stdout:10: error: Need Doxygen comment for any declaration of this entity. [another_print()] (Rule IISB-DoxygenCommentAtDeclaration)
};


template <class T>
void Dummy<T>::print()
{
   do_something();
}

template <class T>
void Dummy<T>::another_print()
{
   do_something();
}

/** template class documentation */
template<typename C>
class XY
{
public:
 /** in-class documentation */
 XY(const XY& other);
};

template<typename C>
XY<C>::XY(const XY& other)
{
}

void in_header_declared_func();
//^stdout:6: error: Need Doxygen comment for any declaration of this entity. [in_header_declared_func()] (Rule IISB-DoxygenCommentAtDeclaration)


static void in_header_defined_func();
//^stdout:13: error: Need Doxygen comment for any declaration of this entity. [in_header_defined_func()] (Rule IISB-DoxygenCommentAtDeclaration)

static void in_header_defined_func()
{
}

void mixed_declared_func_nok();  // label:mixed_declared_func_nok__decl_in_h
//^stdout:6: error: Need Doxygen comment for any declaration of this entity. [mixed_declared_func_nok()] (Rule IISB-DoxygenCommentAtDeclaration), $(mixed_declared_func_nok__decl_in_cpp):6:

/**
 */
void mixed_declared_func_ok();

/**
 * Type T1.
 */
typedef int T1;

typedef int T2;
//^stdout:13: error: Need Doxygen comment for this entity. [T2] (Rule IISB-DoxygenCommentAtDeclaration)

using T3 = int;
//^stdout:7: error: Need Doxygen comment for this entity. [T3] (Rule IISB-DoxygenCommentAtDeclaration)

/** Type T4. */
using T4 = int;

/** Hallo */
static T3 var_3;
