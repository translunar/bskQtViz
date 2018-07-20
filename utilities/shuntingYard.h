/*
 ISC License

 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */
/*
* shuntingYard.h
*
* University of Colorado, Autonomous Vehicle Systems (AVS) Lab
* Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
* Adapted from https://github.com/bmars/shunting-yard
*/

#ifndef _SHUNTING_YARD_H_
#define _SHUNTING_YARD_H_

#include <stddef.h>
#include "utilities/stack.h"

/* Return types */
#define SUCCESS_EQ             -1
#define SUCCESS                 0
#define ERROR_SYNTAX            1
#define ERROR_SYNTAX_STACK      2
#define ERROR_SYNTAX_OPERAND    3
#define ERROR_RIGHT_PAREN       4
#define ERROR_LEFT_PAREN        5
#define ERROR_UNRECOGNIZED      6
#define ERROR_NO_INPUT          7
#define ERROR_FUNC_UNDEF        8
#define ERROR_FUNC_NOARGS       9
#define ERROR_CONST_UNDEF       10
#define ERROR_MATRIX_SIZE       11

/* Stack flags */
#define FLAG_NONE               0
#define FLAG_BOOL_TRUE          1

/* Error defines */
#define ERROR_WIDTH             80
#define NO_COL_NUM             -1

/* Size of string to hold a hexadecimal representation of double precision
* floating point numbers */
#define DOUBLE_STR_LEN          64
/* Number of digits before showing scientific notation */
#define MIN_E_DIGITS            12

/* Convenience functions */
#define isNumeric(c) ((c >= '0' && c <= '9') || c == '.')
#define isAlpha(c) ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_'))
#define isOperand(c) (isNumeric(c) || isAlpha(c))
#define isOperator(c) (c != '\0' && strchr("+-*/%=^", c) != NULL)
#define isSpace(c) (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r')
#ifndef MIN
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef enum {
    OP_BINARY   = 1 << 0,
    OP_UNARY    = 1 << 1,
    OP_PREFIX   = 1 << 2,
    OP_POSTFIX  = 1 << 3,
    OP_NONE     = 1 << 4
} OperatorTypes_t;

typedef struct {
    char            op;
    short int       prec;
    OperatorTypes_t type;
} Operator_t;

void shuntingYardInitializeConstants(void);
int  shuntingYardPutConstant(const char *key, const char *str);
void shuntingYardFreeConstants(void);

/*
* Function: shungingYardMatrix
* Purpose: Parse a string and do the calculations, assigning values to a matrix
*   Assumes matrix is bracketd, with commas to separate column entries, and semi-colons to separate rows
*   Any d input before the first "[" will interpret matrix as diagonal
*   In the event of an error, will set errno to an error code, print error message,
*   and return without changing results
*/
void shuntingYardMatrix(const char *str, const size_t dim1, const size_t dim2, void *result);

/*
* Function: shuntingYardMatrixFreeMemory
* Purpose: Free up memory used by shuntingYardMatrix, call prior to shuntingYardMatrix returning
*/
void shuntingYardMatrixFreeMemory(double *m_result, char *str);

/*
* Function: shuntingYard
* Purpose: Parse a string and do the calculations.
*   In the event of an error, will set errno to an error code, print error message, and return zero
*/
double shuntingYard(const char *str);

/*
* Function: shuntingYardFreeMemory
* Purpose: Free up memory used by shuntingYard, call prior to shuntingYard returning
*/
void shuntingYardFreeMemory(Stack_t *operands, Stack_t *operators, Stack_t *functions, char *str);

/*
* Function: pushOperand
* Purpose: Push an operand onto the stack and substitute any constants
*/
int pushOperand(char *str, int posA, int posB, Stack_t *operands);

/*
* Function: applyOperator
* Purpose: Apply an operator to the top two operands on the stack
*/
int applyOperator(Operator_t *op, Stack_t *operands);
/*
* Function: applyUnaryOperator
* Purpose: Apply an unary operator to the stack
*/
int applyUnaryOperator(char op, Stack_t *operands);
/*
* Function: applyStackOperators
* Purpose: Apply one or more operators currently on the stack
*/
int applyStackOperators(char op, int unary, Stack_t *operands, Stack_t *operators);
/*
* Function: applyFunction
* Purpose: Apply a function with arguments
*/
int applyFunction(char *func, Stack_t *operands);

/*
* Function: compareOperators
* Purpose: Compares the precedence of two operators
*/
int compareOperators(Operator_t *op1, Operator_t *op2);
/*
* Function: num2str
* Purpose: Convert a number to a character string, for adding to the stack
*/
char *num2str(double num);
/*
* Function: strtodUnalloc
* Purpose: Wrapper around strtod() that also frees the string that was converted
*/
double strtodUnalloc(char *str);
/*
* Function: error
* Purpose: Outputs an error
*/
void error(int type, int colNum, const char *str);

/*
* Function: substr
* Purpose: Return a substring
*/
char *substr(const char *str, int start, size_t len);
/*
* Function: cmpStrNoCase
* Purpose: Compare strings without sensitivity to case
*   Same as strncmp(string1, string2, max(sizeof(string1),sizeof(string2)))
*/
int cmpStrNoCase(const char *string1, const char *string2);

/*
* Function: isUnary
* Purpose: Check if an operator is unary
*/
int isUnary(char op, char prevChar);
/*
* Function: trimDouble
* Purpose: Remove trailing zeros from a double and return it as a string
*/
char *trimDouble(double num);
/*
* Function: trimWhiteSpace
* Purpose: Trim whitespace from the end and beginning of a string
*/
char *trimWhiteSpace(char *str);
/*
* Function: ltrim
* Purpose: Trim whitespace from the beginning of a string
*/
char *ltrim(char *str);
/*
* Function: rtrim
* Purpose: Trim whitespace from the end of a string
*/
char *rtrim(char *str);
/*
* Function: getOperator
* Purpose: Look up an operator and return its struct
*/
Operator_t *getOperator(char op, int unary);

#endif
