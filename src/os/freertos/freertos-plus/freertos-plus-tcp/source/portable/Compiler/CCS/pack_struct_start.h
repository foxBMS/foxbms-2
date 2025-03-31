/**
 *       @file: pack_struct_start.h
 *	   @author: jscott
 *       @date: Feb 1, 2022
 *  @copyright: Hotstart 2022 Hotstart Thermal Management. All Rights Reserved.
 *
 *      Permission is hereby granted, free of charge, to any person obtaining a copy of
 *      this software and associated documentation files (the "Software"), to deal in
 *      the Software without restriction, including without limitation the rights to
 *      use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *      the Software, and to permit persons to whom the Software is furnished to do so,
 *      subject to the following conditions:
 *
 *      The above copyright notice and this permission notice shall be included in all
 *      copies or substantial portions of the Software.
 *
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *      FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *      COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *      IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *      CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *      @brief: The leading compiler directive to pack the following structure to 1 byte
 *
 *      Also suppress an incorrect warning from the CCS compiler:
 *          error #1916-D: definition at end of file not followed by a semicolon or a declarator
 *      Pushes the struct alignment of packed.
 */
#pragma diag_push
#pragma diag_suppress=1916
#pragma pack(push, 1)
