/* This test raises a C:005 error because the #endif is not the last line
 * in this file
 */

#ifndef FOXBMS__C_005_TEST2_H_
#define FOXBMS__C_005_TEST2_H_

#endif /* FOXBMS__C_005_TEST2_H_ */

/* the include guards #endif is not the last line of the file - this is bad */
