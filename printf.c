#include <stdio.h>
#include <stdarg.h>
#include <string.h>

enum state {
    INITIAL,
    READ_FLAGS,
    READ_WIDTH,
    READ_PRECISION,
    READ_VEC_SIZE,
    READ_LENGTH,
    READ_SPECIFIER
};

enum LENGTH {
    LENGTH_DEFAULT, hh, h, hl, l
};

enum SPECIFIERS {
    SPEC_DEFAULT,
    SPEC_D,
    SPEC_I,
    SPEC_U,
    SPEC_O,
    SPEC_LOWER_X,
    SPEC_UPPER_X,
    SPEC_LOWER_F,
    SPEC_UPPER_F,
    SPEC_LOWER_E,
    SPEC_UPPER_E,
    SPEC_LOWER_G,
    SPEC_UPPER_G,
    SPEC_LOWER_A,
    SPEC_UPPER_A,
    SPEC_C,
    SPEC_S,
    SPEC_P
};


typedef struct flags flags;
typedef struct vec_size vec_size;
typedef enum LENGTH length;
typedef enum SPECIFIERS specifier;

typedef enum state state;
static state curState = INITIAL;

struct flags {
    int leftJustify;
    int forcePlusMinus;
    int spacePrefixPositiveNumber;
    int zeroPrefixedOrForceDecimal; //o, x or X specifiers only
    int leftPadWithZeroes;
};

struct vec_size {
    int something;
};

struct printSpecification {
    flags f;
    int width;
    int precision;
    vec_size vs;
    length length;
    specifier s;
};


//%[flags][width][.precision][vectorSize][length]specifier

/*
specifier	Output							Example
d or i		Signed decimal integer					392
u		Unsigned decimal integer				7235
o		Unsigned octal						610
x		Unsigned hexadecimal integer				7fa
X		Unsigned hexadecimal integer (uppercase)		7FA
f		Decimal floating point, lowercase			392.65
F		Decimal floating point, uppercase			392.65
e		Scientific notation (mantissa/exponent), lowercase	3.9265e+2
E		Scientific notation (mantissa/exponent), uppercase	3.9265E+2
g		Use the shortest representation: %e or %f		392.65
G		Use the shortest representation: %E or %F		392.65
a		Hexadecimal floating point, lowercase			-0xc.90fep-2
A	Hexadecimal floating point, uppercase				-0XC.90FEP-2
c	Character							a
s	String of characters						sample
p	Pointer address							b8000000

n	Nothing printed.
The corresponding argument must be a pointer to a signed int.
The number of characters written so far is stored in the pointed location.

%	A % followed by another % character will write a single % to the stream.	%

flags	description
-	Left-justify within the given field width; Right justification is the default (see width sub-specifier).
+	Forces to preceed the result with a plus or minus sign (+ or -) even for positive numbers. By default, only negative numbers are preceded with a - sign.
(space)	If no sign is going to be written, a blank space is inserted before the value.
#	Used with o, x or X specifiers the value is preceeded with 0, 0x or 0X respectively for values different than zero.
Used with a, A, e, E, f, F, g or G it forces the written output to contain a decimal point even if no more digits follow. By default, if no digits follow, no decimal point is written.
0	Left-pads the number with zeroes (0) instead of spaces when padding is specified (see width sub-specifier).

width	description
(number)	Minimum number of characters to be printed. If the value to be printed is shorter than this number, the result is padded with blank spaces. The value is not truncated even if the result is larger.
 *	The width is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted.

.precision	description
.number	For integer specifiers (d, i, o, u, x, X): precision specifies the minimum number of digits to be written. If the value to be written is shorter than this number, the result is padded with leading zeros. The value is not truncated even if the result is longer. A precision of 0 means that no character is written for the value 0.
For a, A, e, E, f and F specifiers: this is the number of digits to be printed after the decimal point (by default, this is 6).
For g and G specifiers: This is the maximum number of significant digits to be printed.
For s: this is the maximum number of characters to be printed. By default all characters are printed until the ending null character is encountered.
If the period is specified without an explicit value for precision, 0 is assumed.
.*	The precision is not specified in the format string, but as an additional integer value argument preceding the argument that has to be formatted.

 */

static char *formatInt(int val) {
    return NULL;
}

static char *formatUInt(int val) {
    return NULL;
}

static void initPrintSpec(struct printSpecification* ps) {
    //memset(ps, 0, sizeof(ps));
    ps->f.forcePlusMinus = 0;
    ps->f.leftJustify = 0;
    ps->f.leftPadWithZeroes = 0;
    ps->f.zeroPrefixedOrForceDecimal = 0;
    ps->f.spacePrefixPositiveNumber = 0;

    ps->length = LENGTH_DEFAULT;

    ps->precision = -1;

    ps->s = SPEC_DEFAULT;

    ps->vs.something = -1;

    ps->width = -1;
}

static unsigned int printChar(char* output, char charToPrint, unsigned int *outputPos, unsigned int outputSize) {
    if (*outputPos >= outputSize) {
        return 0;
    }
    output[(*outputPos)++] = charToPrint;
    return 1;
}

static void padString(char* output, unsigned int *outPos, unsigned int outSize, int existingSize,
        int existingPrefixChars, struct printSpecification *ps) {
    int i;
    unsigned int paddingWritten = 0;
    unsigned int paddedSize = ps->width;
    int paddingAmount = paddedSize - existingSize - existingPrefixChars;
    //    printf("padding amount: %d\n", paddingAmount);

    if (paddingAmount <= 0) {
        return;
    }

    //    printf("Padding existing string of %d chars to %u chars\n", existingSize, paddedSize);

    //if it's right justified, move the existing values up by paddedSize-existingSize characters.
    //If it's left justified, just add trailing spaces
    if (ps->f.leftJustify) {
        //        printf("left justifying\n");
        //Just add trailing paddedSize-existingSize trailing spaces
        for (paddingWritten = 0; paddingWritten < paddingAmount;) {
            paddingWritten += printChar(output, ' ', outPos, outSize);
        }
    } else {
        unsigned int stringStart;
        char paddingChar = ps->f.leftPadWithZeroes && ps->s != SPEC_S ? '0' : ' ';
        //Pad the string to the desired width
        unsigned int startingPos = *outPos;
        for (unsigned int i = 0; i < paddingAmount; i++) {
            paddingWritten += printChar(output, paddingChar, outPos, outSize);
        }
        if (paddingAmount != paddingWritten) {
            //The CL spec lets us get away with undefined behavior when the buffer overflows.
            //In this case, we choose to just not right justify, which preserves the existing value as much as possible.
            //            printf("Ran out of output buffer!\n");
            return;
        }
        //Shift the value over by paddedSize-existingSize characters, starting with the right end.
        //Make sure to skip any existing prefix characters.
        //        printf("starting position: %u\n", startingPos);
        //        printf("buffer before right justifying sub-string: ^%s^\n", output);
        stringStart = startingPos - existingSize;
        for (i = existingSize; i >= 0; i--) {
            int from = stringStart + i;
            int to = from + paddingAmount;
            output[to] = output[from];
            //printf("buffer after moving char %d to %d: ^%s^\n", from, to, output);
        }
        //And then fill the left-hand side with padding
        for (i = 0; i < paddingAmount; i++) {
            output[stringStart + i] = paddingChar;
        }
    }
}

//Returns 1/0 for success/fail, returns value in the 'ret' arg.
//If the value starting at fmt[*fmtPos] is NOT a number, *fmtPos is NOT incremented.

static int readUnsigned(const char* fmt, unsigned int* fmtPos, unsigned int* ret) {
    unsigned int val = 0;
    unsigned int foundValue = 0;

    //While we read a number 0-9, multiply the existing value by 10, and add this one.
    char fmtChar = fmt[*fmtPos];
    while (fmtChar >= '0' && fmtChar <= '9') {
        foundValue = 1;
        val *= 10;
        switch (fmtChar) {
            case '1':
                val += 1;
                break;
            case '2':
                val += 2;
                break;
            case '3':
                val += 3;
                break;
            case '4':
                val += 4;
                break;
            case '5':
                val += 5;
                break;
            case '6':
                val += 6;
                break;
            case '7':
                val += 7;
                break;
            case '8':
                val += 8;
                break;
            case '9':
                val += 9;
                break;
        }
        //We've consumed the existing value, increment position and grab next.
        fmtChar = fmt[++(*fmtPos)];
    }

    if (foundValue) {
        *ret = val;
    }
    return foundValue;
}

static void printString(struct printSpecification *ps, char *output, unsigned int *outPos, size_t outSize, char* string) {
    size_t printed = 0;
    int max = ps->precision;
    if (max == -1) {
        max = outSize - *outPos;
    }

    //TODO: Handle output padding (via width, defaults to right justified, but can be left-justified if flags |= '-'
    //We probably need a generic padding function that can left/right justify any string within a given length.
    //Since we can't necessarily allocate memory, we might need to write the string to the output buffer, and then
    //pad/justify afterwards, although that could be slower than it needs to be.
    //Otherwise, we might need to stack allocate the memory and work within that...

    unsigned int startingPos = *outPos;
    //    printf("printing string: %s\n", string);
    while (printed < max && *outPos < outSize) {
        //Detect end-of-string.
        if (string[printed] == '\0') {
            //            printf("breaking\n");
            break;
        }
        //        printf("printing: %c\n", string[printed]);
        output[(*outPos)++] = string[printed++];
    }
    padString(output, outPos, outSize, *outPos - startingPos, 0, ps);
}

static int printSign(struct printSpecification *ps, char *output, unsigned int *outPos, size_t outSize, int isPositive) {
    if (ps->f.forcePlusMinus || !isPositive) {
        return printChar(output, isPositive ? '+' : '-', outPos, outSize);
    } else if (isPositive && ps->f.spacePrefixPositiveNumber) {
        return printChar(output, ' ', outPos, outSize);
    }
    return 0;
}

static unsigned int printDigit(char *output, unsigned int *outPos, size_t outSize, int digit) {
    char charToPrint;
    if (digit < 0) {
        digit *= -1;
    }
    switch (digit) {
        case 0: charToPrint = '0';
            break;
        case 1: charToPrint = '1';
            break;
        case 2: charToPrint = '2';
            break;
        case 3: charToPrint = '3';
            break;
        case 4: charToPrint = '4';
            break;
        case 5: charToPrint = '5';
            break;
        case 6: charToPrint = '6';
            break;
        case 7: charToPrint = '7';
            break;
        case 8: charToPrint = '8';
            break;
        case 9: charToPrint = '9';
            break;
        default: return 0;
    }
    printChar(output, charToPrint, outPos, outSize);
}

static void reverseString(char* output, unsigned int *outPos, unsigned int startPos) {
    //flip the characters from startPos to outPos;
    char tmp;
    unsigned int strLength = (*outPos) - startPos;
    for (int i = 0; i < strLength / 2; i++) {
        unsigned int laterPos = (*outPos) - i - 1;
        tmp = output[laterPos];
        output[laterPos] = output[startPos + i];
        output[startPos + i] = tmp;
    }
}

static void printUnsigned(char *output, unsigned int *outPos, size_t outSize, unsigned long value, int base) {
    //Print the number in reverse, and then flip the result.
    unsigned int startPos = *outPos;
    do {
        unsigned int printed = printDigit(output, outPos, outSize, value % base);
        if (!printed) break;
        value = value / base;
    } while (value != 0);

    //Flip and pad to the desired width
    reverseString(output, outPos, startPos);
}

unsigned long wrapValueToSize(struct printSpecification *ps, unsigned long value) {
    switch (ps->length) {
        case hh:
            return (unsigned char) value;
        case h:
            return (unsigned short) value;
        case l:
            return value;
        case hl:
        default:
            return (unsigned int) value;
    }
}

static void printOctal(struct printSpecification *ps, char *output, unsigned int *outPos, size_t outSize, unsigned long value) {
    value = wrapValueToSize(ps, value);

    unsigned int startPos = *outPos;
    //Now print the number...
    printUnsigned(output, outPos, outSize, value, 8);
    padString(output, outPos, outSize, (*outPos) - startPos, 0, ps);
}

static void printUnsignedLong(struct printSpecification *ps, char *output, unsigned int *outPos, size_t outSize, unsigned long value) {
    value = wrapValueToSize(ps, value);

    unsigned int startPos = *outPos;
    //Now print the number...
    printUnsigned(output, outPos, outSize, value, 10);
    padString(output, outPos, outSize, (*outPos) - startPos, 0, ps);
}

static void printLong(struct printSpecification *ps, char *output, unsigned int *outPos, size_t outSize, long value) {
    switch (ps->length) {
        case hh:
            value = (long) ((char) value);
            break;
        case h:
            value = (long) ((short) value);
            break;
        case l:
            break;
        case hl:
        default:
            value = (int) value;
            break;
    }

    //Print the sign if necessary.
    int signChars = printSign(ps, output, outPos, outSize, value >= 0);

    //Now print the number...
    //Print the number in reverse, and then flip the result.
    unsigned int startPos = *outPos;
    do {
        unsigned int printed = printDigit(output, outPos, outSize, value % 10);
        if (!printed) break;
        value = value / 10;
    } while (value != 0);

    //Reverse the string we just printed.
    reverseString(output, outPos, startPos);

    padString(output, outPos, outSize, (*outPos) - startPos, signChars, ps);
}

static void nextToken(const char *fmt, unsigned int *fmtPos, char *output, unsigned int *outPos, size_t out_size, va_list args) {
    struct printSpecification ps;
    int progress;
    initPrintSpec(&ps);
    char peek;

    //    printf("nextToken, fmtPos = %u\n", *fmtPos);

    char next = fmt[(*fmtPos)++];

    curState = INITIAL;
    if (next != '%') {
        //print token
        output[(*outPos)++] = next;
        return;
    }
    //    printf("fmt char is %%\n");

    //Peek at next char and see if we got %%, and then just print % and bump fmtPos
    if (fmt[*fmtPos] == '%') {
        //print token
        output[(*outPos)++] = fmt[(*fmtPos)++];
        return;
    }

    curState = READ_FLAGS;
    while (curState == READ_FLAGS) {
        progress = 0;

        peek = fmt[*fmtPos];
        switch (peek) {
            case '-':
                ps.f.leftJustify = 1;
                progress = 1;
                //                printf("left justified flag found\n");
                break;
            case '+':
                ps.f.forcePlusMinus = 1;
                progress = 1;
                //                printf("Forcing +/-\n");
                break;
            case ' ':
                ps.f.spacePrefixPositiveNumber = 1;
                progress = 1;
                //                printf("spacePrefix for positive numbers set\n");
                break;
            case '#':
                ps.f.zeroPrefixedOrForceDecimal = 1;
                progress = 1;
                //                printf("zeroPrefixedOrForceDecimal\n");
                break;
            case '0':
                ps.f.leftPadWithZeroes = 1;
                progress = 1;
                //                printf("Left padding with zeroes\n");
                break;
        }

        if (progress) {
            (*fmtPos)++;
        } else {
            curState = READ_WIDTH;
        }
    }
    if (ps.f.spacePrefixPositiveNumber && ps.f.forcePlusMinus) {
        //If both flags are specified, the space prefix flag is ignored.
        ps.f.spacePrefixPositiveNumber = 0;
    }

    //Get the width. Either a number or '*' which indicates consume the next var-arg value.
    peek = fmt[*fmtPos];
    if (peek == '*') {
        (*fmtPos)++;
        ps.width = va_arg(args, int);
    } else {
        readUnsigned(fmt, fmtPos, &ps.width);
    }
    //    printf("Width = %d\n", ps.width);

    curState = READ_PRECISION;

    peek = fmt[*fmtPos];
    if (peek == '.') {
        peek = fmt[++(*fmtPos)];
        if (peek == '*') {
            (*fmtPos)++;
            ps.precision = va_arg(args, int);
        } else {
            //If just a "." is present, precision defaults to 0.
            if (!readUnsigned(fmt, fmtPos, &ps.precision)) {
                ps.precision = 0;
            }
        }
    }
    //    printf("Precision = %d\n", ps.precision);
    curState = READ_VEC_SIZE;

    while (curState == READ_VEC_SIZE) {
        progress = 0;
        if (!progress)
            curState = READ_LENGTH;
        break;
    }

    ps.length = LENGTH_DEFAULT;
    while (curState == READ_LENGTH) {
        progress = 0;

        peek = fmt[*fmtPos];
        if (peek == 'h') {
            switch (ps.length) {
                case LENGTH_DEFAULT:
                    ps.length = h;
                    progress = 1;
                    break;
                case h:
                    ps.length = hh;
                    progress = 1;
                    break;
            }
        }
        if (peek == 'l') {
            switch (ps.length) {
                case LENGTH_DEFAULT:
                    ps.length = l;
                    progress = 1;
                    break;
                case h:
                    ps.length = hl;
                    progress = 1;
                    break;
            }
        }

        //Read length field. Could be v2, v3, v4, v8, v16
        //Or could be hh, h, hl (only for vectors), l
        //For int's that means: char, short, int, long

        if (progress)
            (*fmtPos)++;
        else
            curState = READ_SPECIFIER;
    }

    if (curState == READ_SPECIFIER) {
        char specifier = fmt[(*fmtPos)++];
        //All: d, i, u, o, x, X, f, F, e, E, g, G, a, A, c, s, p
        //TODO: x, X, f, F, e, E, g, G, a, A, p, vN
        //DONE: d, i, u, c, s, o,
        switch (specifier) {
            case 's':
                ps.s = SPEC_S;
                printString(&ps, output, outPos, out_size, va_arg(args, char*));
                break;
            case 'c':
                ps.s = SPEC_C;
                printChar(output, va_arg(args, int), outPos, out_size);
                break;
            case 'o':
                ps.s = SPEC_O;
                if (ps.length != l)
                    printOctal(&ps, output, outPos, out_size, (unsigned long) va_arg(args, unsigned int));
                else
                    printOctal(&ps, output, outPos, out_size, (unsigned long) va_arg(args, unsigned long));
                break;
            case 'd':
            case 'i':
                ps.s = SPEC_D;
                if (ps.length != l)
                    printLong(&ps, output, outPos, out_size, (long) va_arg(args, int));
                else
                    printLong(&ps, output, outPos, out_size, (long) va_arg(args, long));
                break;
            case 'u':
                ps.s = SPEC_U;
                if (ps.length != l)
                    printUnsignedLong(&ps, output, outPos, out_size, (unsigned long) va_arg(args, unsigned int));
                else
                    printUnsignedLong(&ps, output, outPos, out_size, (unsigned long) va_arg(args, unsigned long));
                break;
        }
    } else {
        printf("ERROR: I should be reading a specifier here... but I'm not\n");
    }
}

static int myPrintf(char* output, size_t out_size, const char* fmt, va_list args) {
    unsigned int fmtPos = 0;
    unsigned int outPos = 0;

    //    printf("format is: %s", fmt);

    //Get the next argument as an integer
    //va_arg(args, int);

    //While output has written less than size and fmt string is not exhausted
    //Read next char
    //If in initial state and char is not %, print that char.
    //Might need to deal with escaped characters
    //If next char is %, go to READING_FLAGS
    //Read flags of -+<space>#0 for:
    //  "-" left justify within field width
    //  "+" force display of "+"/"-" , even for negatives. Default is just "-" for negatives
    //  <space> If no sign is written, insert blank space before value.
    //  "#" controls 0, 0x, 0X prefix for hex values, forces decimal point for floats/doubles even if no trailing decimal value.
    //  "0" Left pad with zeros to specified width value
    //
    //If something else read, then move state to READ_WIDTH
    //  (number) Read a number of characters of field width.
    //  "*" Read the number of characters of width from the next unused argument
    //
    //Next, move state to READ_PRECISION
    //  .(number) For integers, minimum number of digits (leading zero padding)
    //            For floats, number of digits after decimal point (default 6).
    //
    //Move to READ_VECTOR_SIZE
    //  vN - Size of vector argument to print
    //
    //Next, move to READ_LENGTH
    //  values: none, hh, h, hl (only valid for vector specifiers), l
    // Any values printed to a given length must be cast before printing. Implicit cast between vector types are not allowed.
    //
    //Lastly, move to READ_SPECIFIER
    //  values: d, i, u, x, X, f, F, e, E, g, G, a, A, c, s, p, n

    char nextChar;
    while ((nextChar = fmt[fmtPos]) != 0 && outPos < out_size) {
        nextToken(fmt, &fmtPos, output, &outPos, out_size, args);
        //        printf("End of printf loop iteration. fmtPos = %u, outPos = %u\n", fmtPos, outPos);
    }
    //    printf("Done with printf's while loop\n");

    //Always null-terminate the output buffer.
    if (outPos < out_size - 1) {
        output[outPos] = '\0';
    } else {
        output[outPos - 1 - 1] = '\0';
    }

    //What would we consider a non-successful printf?
    //Running out of space in the buffer? Invalid format/#(arguments)?
    return 1;
}

int testPattern(char *buffer, size_t buffer_size, char* fmt, ...) {
    char cpuOutput[buffer_size];

    //First, clear the output buffer.
    memset(buffer, 0, sizeof (buffer));

    va_list args;

    va_start(args, fmt);
    vsprintf(cpuOutput, fmt, args);
    va_end(args);

    va_start(args, fmt);
    myPrintf(buffer, buffer_size, fmt, args);
    va_end(args);

    if (strcmp(cpuOutput, buffer)) {
        printf("Difference between system and myPrintf for pattern:\n%s\n", fmt);
        printf("System:%s\nprintf:%s\n\n", cpuOutput, buffer);
    } else {
        printf("Correct result. Buffer: %s\n\n", buffer);
    }
    return strcmp(cpuOutput, buffer);
}

int main() {
    char buffer[1024];
    size_t bufSize = sizeof (buffer);
    testPattern(buffer, bufSize, "hello%%, :%010.7s%s:           asdfasdf\n", "world..........", "");

    testPattern(buffer, bufSize, ":%07.10s:%c:%d:%+d:%i\n", "hello", 'T', 1, 1234, -1024);

    testPattern(buffer, bufSize, ":%hhd:%hd:%d:%ld:\n", 128, 32768, 65536, 4294967295);

    printf("Expect the following to fail. ");
    printf("system printf seems to grant precedence to the '0' flag, not the '-' flag.\n");
    testPattern(buffer, bufSize, ":%-0.7d:\n", 32768);
    //Both rules should be part of a number padding function:
    //If the 0 flag was specified, pad with zeroes to field width, If '0' and '-' flags are both specified, ignore '0'.
    //If a precision is specified, the '0' flag is ignored.

    testPattern(buffer, bufSize, ":%hhd:%hd:%d:%ld:\n", 128, 32768, 65536, 4294967295);

    //Right justified string
    testPattern(buffer, bufSize, "^%10s^", "test");
    //Right justified integer
    testPattern(buffer, bufSize, "^%10d^", 10);
    //Right justified integer with leading zeroes
    testPattern(buffer, bufSize, "^%010d^", 10);
    //Right justified integer with leading zeroes and sign prefix
    testPattern(buffer, bufSize, "^%+010d^", 10);
    //Left justified string
    testPattern(buffer, bufSize, "^%-10s^", "test");
    //Left justified integer.
    testPattern(buffer, bufSize, "^%-10d^", 10);

    //Unsigned char, short, int, long values that are just over the equivalent signed rollover.
    testPattern(buffer, bufSize, "^%hhu^", 128);
    testPattern(buffer, bufSize, "^%hu^", 32768);
    testPattern(buffer, bufSize, "^%u^", (unsigned int) 2147483648);
    testPattern(buffer, bufSize, "^%lu^", 9223372036854775808LU);

    //Unsigned Octal
    testPattern(buffer, bufSize, "^%hho^", 128);
    testPattern(buffer, bufSize, "^%ho^", 32768);
    testPattern(buffer, bufSize, "^%o^", (unsigned int) 2147483648);
    testPattern(buffer, bufSize, "^%lo^", 9223372036854775808LU);

    //Character
    testPattern(buffer, bufSize, "^%c%c%c%c%c^", 'h', 'e', 'l', 'l', 'o');

}
