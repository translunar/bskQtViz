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
*  shuntingYard.c
*
* University of Colorado, Autonomous Vehicle Systems (AVS) Lab
* Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
*/

#include "shuntingYard.h"

#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "utilities/astroConstants.h"
#include "strMap.h"

#define MXINDEX(dim2, row, col) ((row)*(dim2) + (col))
#define ALLOC_DOUBLE(dim) ((double*)calloc(dim, sizeof(double)))
#define FREE_DOUBLE(arr) (free(arr))
#define MOVE_DOUBLE(source, dim, destination) (memmove((void*)(destination), (void*)(source), sizeof(double)*(dim)))

/* Suppress error output when true */
int syQuiet = false;

static Operator_t ops[] = {
    {'^', 6, OP_BINARY},

    {'+', 4, (OperatorTypes_t)(OP_UNARY | OP_PREFIX)},
    {'-', 4, (OperatorTypes_t)(OP_UNARY | OP_PREFIX)},

    {'*', 3, OP_BINARY},
    {'/', 3, OP_BINARY},
    {'+', 2, OP_BINARY},
    {'-', 2, OP_BINARY},
    {'%', 2, OP_BINARY},
    {'=', 1, OP_BINARY},

    {'(', 0, OP_NONE},
    {')', 0, OP_NONE}
};

static StrMap_t *constantsMap;

void shuntingYardInitializeConstants(void)
{
    constantsMap = strMapAlloc(1024);
    strMapPut(constantsMap, "e", M_E);
    strMapPut(constantsMap, "pi", M_PI);
    strMapPut(constantsMap, "tau", 2.0 * M_PI);
    strMapPut(constantsMap, "REQ_EARTH", REQ_EARTH);
}

int shuntingYardPutConstant(const char *key, const char *str)
{
    double value = shuntingYard(str);
    return strMapPut(constantsMap, key, value);
}

void shuntingYardFreeConstants(void)
{
    strMapFree(constantsMap);
}

void shuntingYardMatrix(const char *inputStr, const size_t dim1, const size_t dim2, void *result)
{
    int     i;
    int     init = 0;
    int     sy0 = 0;
    size_t  j = 0;
    size_t  k = 0;
    size_t  jj;
    size_t  kk;
    double *m_result = ALLOC_DOUBLE(dim1 * dim2);
    char   *str = (char *)calloc(strlen(inputStr) + 1, sizeof(char));
    int     isDiag = 0;

    for(i = 0; i <= (int)strlen(inputStr); i++) {
        if(!init) {
            if(inputStr[i] == '[') {
                sy0 = i + 1;
                init = 1;
            } else if(inputStr[i] == 'd') {
                isDiag = 1;
                if(dim1 == 1 || dim2 == 1) {
                    error(ERROR_MATRIX_SIZE, i, inputStr);
                    shuntingYardMatrixFreeMemory(m_result, str);
                    return;
                }
                for(jj = 0; jj < dim1; jj++) {
                    for(kk = 0; kk < dim2; kk++) {
                        m_result[MXINDEX(dim2, j, k)] = 0.0;
                    }
                }
            }
        } else {
            if(inputStr[i] == '[') {
                error(ERROR_SYNTAX, i, inputStr);
                shuntingYardMatrixFreeMemory(m_result, str);
                return;
            } else if(inputStr[i] == ',' || inputStr[i] == ';' || inputStr[i] == ']') {
                strncpy(str, &inputStr[sy0], i - sy0);
                str[i - sy0] = '\0';
                if(isDiag) {
                    m_result[MXINDEX(dim2, j, j)] = shuntingYard(str);
                } else {
                    m_result[MXINDEX(dim2, j, k)] = shuntingYard(str);
                }
                if(errno != 0) {
                    error(errno, i, inputStr);
                    shuntingYardMatrixFreeMemory(m_result, str);
                    return;
                }
                sy0 = i + 1;
                j++;

                if(inputStr[i] == ',') {
                    if(j == dim1) {
                        error(ERROR_MATRIX_SIZE, i, inputStr);
                        shuntingYardMatrixFreeMemory(m_result, str);
                        return;
                    }
                } else if(inputStr[i] == ';') {
                    if(isDiag) {
                        error(ERROR_SYNTAX, i, inputStr);
                        shuntingYardMatrixFreeMemory(m_result, str);
                        return;
                    }
                    if(j == dim1) {
                        k++;
                        j = 0;
                    } else {
                        error(ERROR_MATRIX_SIZE, i, inputStr);
                        shuntingYardMatrixFreeMemory(m_result, str);
                        return;
                    }
                } else {
                    if(isDiag) {
                        if(j != dim1) {
                            error(ERROR_MATRIX_SIZE, i, inputStr);
                            shuntingYardMatrixFreeMemory(m_result, str);
                            return;
                        }
                    } else {
                        if(j != dim1 || k != dim2 - 1) {
                            error(ERROR_MATRIX_SIZE, i, inputStr);
                            shuntingYardMatrixFreeMemory(m_result, str);
                            return;
                        }
                    }
                }
            }
        }
    }

    MOVE_DOUBLE(m_result, dim1 * dim2, result);
    shuntingYardMatrixFreeMemory(m_result, str);
}

void shuntingYardMatrixFreeMemory(double *m_result, char *str)
{
    FREE_DOUBLE(m_result);
    free(str);
}

double shuntingYard(const char *inputStr)
{
    double       result = 0;
    Stack_t     *operands = stackAlloc();
    Stack_t     *operators = stackAlloc();
    Stack_t     *functions = stackAlloc();
    StackItem_t *item;

    // Loop variables
    int          i;
    int          tokenPos = -1;
    int          parenDepth = 0;
    char         prevChar = '\0';
    char        *operand;

    // Variables used only for error() - not required for parsing
    int          errorType = 0;
    int          parenPos = -1;

    // Make local copy of input str
    char        *str = (char *)calloc(strlen(inputStr) + 1, sizeof(char));
    strcpy(str, inputStr);

    errno = 0;

    // Hack to replace "e" with "*10^" when appropriate
    for(i = 0; i <= (int)strlen(str); i++) {
        if(str[i] == 'e' || str[i] == 'E') {
            if(i > 0 && isNumeric(str[i - 1])
                    && (isNumeric(str[i + 1]) || str[i + 1] == '-' || str[i + 1] == '+')) {
                char *newStr = (char *)calloc(strlen(str) + 4, sizeof(char));
                strncpy(newStr, str, i);
                newStr[i] = '\0';
                strcat(newStr, "*10^");
                strcat(newStr, str + i + 1);
                str = (char *)realloc(str, strlen(newStr) + 1);
                if(str != NULL) {
                    strcpy(str, newStr);
                    free(newStr);
                } else {
                    free(newStr);
                    error(ERROR_SYNTAX, 0, str);
                    shuntingYardFreeMemory(operands, operators, functions, str);
                    return result;
                }
            }
        }
    }

    // Loop through expression
    for(i = 0; i <= (int)strlen(str); i++) {
        // Convert char to char*
        char charStr[2];
        charStr[0] = str[i];
        charStr[1] = '\0';

        if(str[i] == ' ') {
            continue;
        }

        // Operands
        if(isOperand(str[i])) {
            if(tokenPos == -1) {
                // If first char found, keep going until end of operand found
                tokenPos = i;
            } else if(isAlpha(str[i]) && isNumeric(prevChar)) {
                // Parse expressions like "2a"
                if(!pushOperand(str, tokenPos, i, operands)) {
                    shuntingYardFreeMemory(operands, operators, functions, str);
                    return result;
                }
                tokenPos = i;

                // Emulate encountering a "*" operator, since "2a" implies "2*a"
                // Apply any lower precedence operators on the stack first
                if(!applyStackOperators('*', false, operands, operators)) {
                    error(ERROR_SYNTAX, i, str);
                    shuntingYardFreeMemory(operands, operators, functions, str);
                    return result;
                }
                // then push '*' onto stack
                stackPush(operators, "*", false);

            } else if(isNumeric(str[i]) && isAlpha(prevChar)) {
                // "a2" instead of "2a" is invalid
                error(ERROR_SYNTAX, i, str);
                shuntingYardFreeMemory(operands, operators, functions, str);
                return result;
            }

            if(str[i] == '\n') {
                break;
            }
            prevChar = str[i];
            continue;

        } else if(tokenPos != -1) {
            // If we found a non operand, try and push the operand found so far
            if(!pushOperand(str, tokenPos, i, operands)) {
                shuntingYardFreeMemory(operands, operators, functions, str);
                return result;
            }
            tokenPos = -1;
        }

        // Operators
        if(isOperator(str[i])) {
            int unary = isUnary(str[i], prevChar);

            // Apply any lower precedence operators on the stack first
            if(!applyStackOperators(str[i], unary, operands, operators)) {
                error(ERROR_SYNTAX, i, str);
                shuntingYardFreeMemory(operands, operators, functions, str);
                return result;
            }

            // Push current operator
            stackPush(operators, charStr, unary);
        } else if(str[i] == '(') {
            // Check if this paren is starting a function
            if(isOperand(prevChar)) {
                stackPushUnalloc(functions, stackPop(operands), FLAG_NONE);
            }
            stackPush(operators, charStr, 0);
            parenDepth++;
            if(parenDepth == 1) {
                parenPos = i;
            }

        } else if(str[i] == ')') {
            // If we find an unmatched ')' throw an error
            if(!parenDepth) {
                error(ERROR_RIGHT_PAREN, i, str);
                shuntingYardFreeMemory(operands, operators, functions, str);
                return result;
            }

            // Pop and apply operators until we reach the left paren
            while(!stackIsEmpty(operators)) {
                // We found a left paren
                if(stackTop(operators)[0] == '(') {
                    stackPopChar(operators);
                    parenDepth--;
                    break;
                }

                item = stackTopItem(operators);
                // Try and apply operator on the top of the stack
                if(!applyOperator(getOperator(item->val[0], item->flags), operands)) {
                    error(ERROR_SYNTAX, i, str);
                    shuntingYardFreeMemory(operands, operators, functions, str);
                    return result;
                }
                free(stackPop(operators));
            }

            // Check if this is the end of a function
            if(!stackIsEmpty(functions)) {
                operand = stackPop(functions);
                errorType = applyFunction(operand, operands);
                free(operand);
                if(errorType != SUCCESS) {
                    error(errorType, i, str);
                    shuntingYardFreeMemory(operands, operators, functions, str);
                    return result;
                }
            }

        } else if(str[i] != '\0' && str[i] != '\n') {
            // Unknown character encountered
            error(ERROR_UNRECOGNIZED, i, str);
            shuntingYardFreeMemory(operands, operators, functions, str);
            return result;
        }

        // If we've reached the end
        if(str[i] == '\0' || str[i] == '\n') {
            break;
        }
        prevChar = str[i];
    }

    // If there are still parens on the stack we have an unmatched paren
    if(parenDepth) {
        error(ERROR_LEFT_PAREN, parenPos, str);
        shuntingYardFreeMemory(operands, operators, functions, str);
        return result;
    }

    // End of string, so apply any remaining operators on the stack
    while(!stackIsEmpty(operators)) {
        item = stackTopItem(operators);
        // Try and apply operator on the top of the stack
        if(!applyOperator(getOperator(item->val[0], item->flags), operands)) {
            error(ERROR_SYNTAX_STACK, NO_COL_NUM, str);
            shuntingYardFreeMemory(operands, operators, functions, str);
            return result;
        }
        free(stackPop(operators));
    }

    // Save the final result
    if(stackIsEmpty(operands)) {
        error(ERROR_NO_INPUT, NO_COL_NUM, str);
    } else {
        item = stackTopItem(operands);

        // Convert equations into a boolean result
        if(errno == SUCCESS_EQ) {
            result = (item->flags == FLAG_BOOL_TRUE ? 1.0 : 0.0);
        } else {
            result = strtod(item->val, NULL);
        }
    }

    // Free memory and return
    shuntingYardFreeMemory(operands, operators, functions, str);
    return result;
}

void shuntingYardFreeMemory(Stack_t *operands, Stack_t *operators, Stack_t *functions, char *str)
{
    stackFree(operands);
    stackFree(operators);
    stackFree(functions);
    free(str);
}

int pushOperand(char *str, int posA, int posB, Stack_t *operands)
{
    char *operand = rtrim(substr(str, posA, (size_t)(posB - posA)));
    // Syntax chech. Error if one of the following is true:
    //  1. Operand ONLY contains "."
    //  2. Operand contains a space
    //  3. Operand contains more than one "."
    int onlyPeriod = strcmp(operand, ".") == 0;
    int containsSpace = strchr(operand, ' ') != NULL;
    int multiplePeriods = false;
    char *period = strchr(operand, '.');
    if(period != NULL) {
        multiplePeriods = strchr(period + 1, '.') != NULL;
    }

    if(onlyPeriod || containsSpace || multiplePeriods) {
        error(ERROR_SYNTAX_OPERAND, posA, str);
        free(operand);
        return false;
    }

    // Substitute constants
    if(isAlpha(operand[0])) {
        if(strMapExists(constantsMap, operand)) {
            double value;
            strMapGet(constantsMap, operand, &value);
            operand = num2str(value);
        } else if(str[posB] != '(') {
            // Unknown constant
            error(ERROR_CONST_UNDEF, posA, str);
            free(operand);
            return false;
        }
    }

    stackPushUnalloc(operands, operand, FLAG_NONE);
    return true;
}

int applyOperator(Operator_t *op, Stack_t *operands)
{
    short int flags = FLAG_NONE;
    double result;
    double val1;
    double val2;

    // Check for null op or underfloas, as it indicates a syntax error
    if(op == NULL || stackIsEmpty(operands)) {
        return false;
    }

    // Apply an unary operaotr
    if(op->type & OP_UNARY) {
        return applyUnaryOperator(op->op, operands);
    }

    val2 = strtodUnalloc(stackPop(operands));
    // Check for underflow again before we pop another operand
    if(stackIsEmpty(operands)) {
        return false;
    }
    val1 = strtodUnalloc(stackPop(operands));

    switch(op->op) {
        case '+':
            result = val1 + val2;
            break;
        case '-':
            result = val1 - val2;
            break;
        case '*':
            result = val1 * val2;
            break;
        case '/':
            result = val1 / val2;
            break;
        case '%':
            result = fmod(val1, val2);
            break;
        case '^':
            result = pow(val1, val2);
            break;
        case '=':
            // Indicate that output is now a boolean instead of a number
            errno = SUCCESS_EQ;
            if(fabs(val1 - val2) == 0) {
                // Operator returns original value
                result = val1;
                // This is used instead of simple typecasting the result into a
                // int later on, because that would case "0=0" to return false
                flags = FLAG_BOOL_TRUE;
            } else {
                result = 0.0;
            }
            break;
        default:
            return false;
    }

    stackPushUnalloc(operands, num2str(result), flags);
    return true;
}

int applyUnaryOperator(char op, Stack_t *operands)
{
    double result;
    double val = strtodUnalloc(stackPop(operands));

    switch(op) {
        case '+':
            // Values are assumed positive
            result = val;
            break;
        case '-':
            result = -val;
            break;
        default:
            // Unknown operator
            return false;
    }

    stackPushUnalloc(operands, num2str(result), FLAG_NONE);
    return true;
}

int applyStackOperators(char op, int unary, Stack_t *operands, Stack_t *operators)
{
    // Loop through the oeprator stack and apply operators until we reach one
    // that's of lower precedence (with different rules for unary operators)
    StackItem_t *item;
    Operator_t  *op1;
    Operator_t  *op2;
    while(!stackIsEmpty(operators)) {
        item = stackTopItem(operators);

        op1 = getOperator(item->val[0], item->flags);
        op2 = getOperator(op, unary);
        if(!compareOperators(op1, op2)) {
            break;
        }

        if(!applyOperator(getOperator(item->val[0], item->flags), operands)) {
            return false;
        }
        free(stackPop(operators));
    }
    return true;
}

int applyFunction(char *func, Stack_t *operands)
{
    double arg;
    double result;

    // Function arguments can't be void
    if(stackIsEmpty(operands)) {
        return ERROR_FUNC_NOARGS;
    }

    // Pop the last operand from the stack and use it as the argument
    arg = strtodUnalloc(stackPop(operands));

    if(cmpStrNoCase(func, "abs") == 0) {
        result = fabs(arg);
    } else if(cmpStrNoCase(func, "sqrt") == 0) {
        result = sqrt(arg);
    } else if(cmpStrNoCase(func, "ln") == 0) {
        result = log(arg);
    } else if(cmpStrNoCase(func, "lb") == 0) {
        result = log2(arg);
    } else if(cmpStrNoCase(func, "lg") == 0
              || cmpStrNoCase(func, "log") == 0) {
        result = log10(arg);
    } else if(cmpStrNoCase(func, "cos") == 0) {
        result = cos(arg);
    } else if(cmpStrNoCase(func, "acos") == 0) {
        result = acos(arg);
    } else if(cmpStrNoCase(func, "sin") == 0) {
        result = sin(arg);
    } else if(cmpStrNoCase(func, "asin") == 0) {
        result = asin(arg);
    } else if(cmpStrNoCase(func, "tan") == 0) {
        result = tan(arg);
    } else if(cmpStrNoCase(func, "atan") == 0) {
        result = atan(arg);
    } else if(cmpStrNoCase(func, "cosh") == 0) {
        result = cosh(arg);
    } else if(cmpStrNoCase(func, "sinh") == 0) {
        result = sinh(arg);
    } else if(cmpStrNoCase(func, "tanh") == 0) {
        result = tanh(arg);
    } else if(cmpStrNoCase(func, "exp") == 0) {
        result = exp(arg);
    } else {
        return ERROR_FUNC_UNDEF;
    }

    stackPushUnalloc(operands, num2str(result), FLAG_NONE);
    return SUCCESS;
}

int compareOperators(Operator_t *op1, Operator_t *op2)
{
    if(op1 == NULL || op2 == NULL) {
        return -1;
    }
    // unary operators have special precedence
    return op1->prec >= op2->prec && (op2->type == OP_BINARY);
}

char *num2str(double num)
{
    char *str = (char *)calloc(DOUBLE_STR_LEN, sizeof(char));
    sprintf(str, "%.*g", DBL_DIG, num);
    return str;
}

double strtodUnalloc(char *str)
{
    double num = strtod(str, NULL);
    free(str);
    return num;
}

void error(int type, int colNum, const char *str)
{
    int  totalWidth;
    int  msgWidth;
    int  availableWidth;
    char errorStr[80] = "Error: ";
    errno = type;
    if(syQuiet) {
        // Suppress error output
        return;
    }

    switch(type) {
        case ERROR_SYNTAX:
        case ERROR_SYNTAX_STACK:
        case ERROR_SYNTAX_OPERAND:
            strcat(errorStr, "malformed expression");
            break;
        case ERROR_RIGHT_PAREN:
            strcat(errorStr, "mismatched right parenthesis");
            break;
        case ERROR_LEFT_PAREN:
            strcat(errorStr, "mismatched (unclosed) left parenthesis");
            break;
        case ERROR_UNRECOGNIZED:
            strcat(errorStr, "unrecognized character");
            break;
        case ERROR_NO_INPUT:
            strcat(errorStr, "no input");
            break;
        case ERROR_FUNC_UNDEF:
            strcat(errorStr, "undefined function");
            break;
        case ERROR_FUNC_NOARGS:
            strcat(errorStr, "function requires arguments");
            break;
        case ERROR_CONST_UNDEF:
            strcat(errorStr, "undefined constant");
            break;
        case ERROR_MATRIX_SIZE:
            strcat(errorStr, "matrix dimensions not consistent");
            break;
        default:
            strcat(errorStr, "unknown error");
    }

    // Output exerpt and column marker
    totalWidth = ERROR_WIDTH;
    msgWidth = (int)strlen(errorStr);
    availableWidth = MIN(totalWidth - msgWidth, (int)strlen(str) * 2);
    if(colNum != NO_COL_NUM) {
        int   substrStart = MAX(colNum - availableWidth / 2, 0);
        char *excerpt = substr(str, substrStart, (size_t)availableWidth);

        strcat(errorStr, ": ");
        msgWidth += 2;

        // Width variables start at 1
        colNum++;
        fprintf(stderr, "%s%s\n", errorStr, excerpt);
        fprintf(stderr, "%*c\n", msgWidth + colNum - substrStart, '^');
        free(excerpt);
    } else {
        if(type != ERROR_NO_INPUT) {
            char *excerpt = substr(str, 0, (size_t)availableWidth);
            fprintf(stderr, "%s: %s\n", errorStr, excerpt);
        } else {
            fprintf(stderr, "%s\n", errorStr);
        }
    }
}

char *substr(const char *str, int start, size_t len)
{
    char *substring = (char *)calloc(len + 1, sizeof(char));
    memcpy(substring, str + start, len);
    substring[len] = '\0';
    return substring;
}

int cmpStrNoCase(const char *string1, const char *string2)
{
    size_t  string1len = strlen(string1);
    size_t  string2len = strlen(string2);
    char   *string1lower = (char *)calloc(string1len, sizeof(char));
    char   *string2lower = (char *)calloc(string2len, sizeof(char));
    size_t  i;

    memcpy(string1lower, string1, string1len);
    memcpy(string2lower, string2, string2len);

    for(i = 0; i < string1len; i++) {
        string1lower[i] = (char)tolower(string1lower[i]);
    }
    for(i = 0; i < string2len; i++) {
        string2lower[i] = (char)tolower(string2lower[i]);
    }

    return strncmp(string1lower, string2lower, string1len > string2len ? string1len : string2len);
}

int isUnary(char op, char prevChar)
{
    if(strchr("+-", op) != NULL) {
        // Left paren counts as an operand for prefix operators
        return isOperator(prevChar) || prevChar == '\0' || prevChar == '(';
    } else {
        return false;
    }
}

char *trimDouble(double num)
{
    size_t  i;
    char   *str = (char *)calloc(DBL_DIG + 1, sizeof(char));

    sprintf(str, num >= pow(10.0, MIN_E_DIGITS) ?
            "%.*e" : "%.*f", MIN_E_DIGITS, num);

    for(i = strlen(str) - 1; i > 0; i--) {
        if(str[i] == '.') {
            str[i] = '\0';
        }
        if(str[i] != '0') {
            break;
        }
        str[i] = '\0';
    }

    return NULL;
}

char *trimWhiteSpace(char *str)
{
    char *end;

    /* Trim leading space */
    while(isspace(*str)) {
        str++;
    }

    if(*str == 0) { // All spaces?
        return str;
    }

    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) {
        end--;
    }
    /* Write new null terminator */
    *(end + 1) = 0;

    return str;
}

char *ltrim(char *str)
{
    /* Trim leading space */
    while(isspace(*str)) {
        str++;
    }
    return str;
}

char *rtrim(char *str)
{
    /* Trim trailing space */
    char *end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) {
        end--;
    }
    /* Write new null terminator */
    *(end + 1) = '\0';
    return str;
}

Operator_t *getOperator(char op, int unary)
{
    size_t i;

    for(i = 0; i < sizeof(ops) / sizeof(ops[0]); i++) {
        int opEqual = ops[i].op == op;
        int unaryEqual = unary == ((ops[i].type & OP_UNARY) == OP_UNARY);
        if(opEqual && unaryEqual) {
            return &ops[i];
        }
    }

    return NULL;
}
