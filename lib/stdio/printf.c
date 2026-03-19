#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int isdigit(char c)
{
    return (c >= '0' && c <= '9');
}

char* __int_str(intmax_t i, char b[], int base, bool plusSignIfNeeded,
                bool spaceSignIfNeeded, int paddingNo,
                bool justify, bool zeroPad)
{
    char digit[32] = {0};
    strcpy(digit, "0123456789");

    if (base == 16)
        strcat(digit, "ABCDEF");
    else if (base == 17)
    {
        strcat(digit, "abcdef");
        base = 16;
    }

    char *p = b;

    if (i < 0)
    {
        *p++ = '-';
        i *= -1;
    }
    else if (plusSignIfNeeded)
    {
        *p++ = '+';
    }
    else if (spaceSignIfNeeded)
    {
        *p++ = ' ';
    }

    intmax_t shifter = i;

    do
    {
        ++p;
        shifter /= base;
    } while (shifter);

    *p = '\0';

    do
    {
        *--p = digit[i % base];
        i /= base;
    } while (i);

    int padding = paddingNo - strlen(b);
    if (padding < 0)
        padding = 0;

    if (justify)
    {

        while (padding--)
            b[strlen(b)] = zeroPad ? '0' : ' ';
    }
    else
    {

        char a[256] = {0};

        while (padding--)
            a[strlen(a)] = zeroPad ? '0' : ' ';

        strcat(a, b);
        strcpy(b, a);
    }

    return b;
}

typedef void (*out_f)(char c, void *ctx);

static void out_char(out_f out, void *ctx, char c, int *count)
{
    out(c, ctx);
    (*count)++;
}

static void out_string(out_f out, void *ctx, const char *s, int *count)
{
    while (*s)
        out_char(out, ctx, *s++, count);
}

int vformat(out_f out, void *ctx, const char *format, va_list list)
{
    int chars = 0;
    char intStrBuffer[256];

    for (int i = 0; format[i]; i++)
    {

        if (format[i] != '%')
        {
            out_char(out, ctx, format[i], &chars);
            continue;
        }

        i++;

        bool leftJustify = false;
        bool zeroPad = false;
        bool spaceNoSign = false;
        bool altForm = false;
        bool plusSign = false;

        int width = 0;
        int precision = 0;

        char length = 0;
        char specifier;

        bool parsing = true;

        while (parsing)
        {

            switch (format[i])
            {

            case '-':
                leftJustify = true;
                i++;
                break;
            case '+':
                plusSign = true;
                i++;
                break;
            case '#':
                altForm = true;
                i++;
                break;
            case ' ':
                spaceNoSign = true;
                i++;
                break;
            case '0':
                zeroPad = true;
                i++;
                break;

            default:
                parsing = false;
                break;
            }
        }

        while (isdigit(format[i]))
        {
            width = width * 10 + (format[i] - '0');
            i++;
        }

        if (format[i] == '*')
        {
            width = va_arg(list, int);
            i++;
        }

        if (format[i] == '.')
        {

            i++;
            precision = 0;

            while (isdigit(format[i]))
            {
                precision = precision * 10 + (format[i] - '0');
                i++;
            }

            if (format[i] == '*')
            {
                precision = va_arg(list, int);
                i++;
            }
        }

        if (strchr("hljztL", format[i]))
        {

            length = format[i];
            i++;

            if (length == 'h' && format[i] == 'h')
            {
                length = 'H';
                i++;
            }

            if (length == 'l' && format[i] == 'l')
            {
                length = 'q';
                i++;
            }
        }

        specifier = format[i];

        memset(intStrBuffer, 0, sizeof(intStrBuffer));

        int base = 10;

        if (specifier == 'o')
        {
            base = 8;
            specifier = 'u';

            if (altForm)
                out_string(out, ctx, "0", &chars);
        }

        if (specifier == 'p')
        {

            base = 16;
            specifier = 'u';
            length = 'z';

            out_string(out, ctx, "0x", &chars);
        }

        if (specifier == 'd' || specifier == 'i' ||
            specifier == 'u' || specifier == 'x' || specifier == 'X')
        {
            if (specifier == 'x')
                base = 17;
            if (specifier == 'X')
                base = 16;

            uintmax_t val = 0;

            switch (length)
            {

            case 'H':
                val = (unsigned char)va_arg(list, int);
                break;
            case 'h':
                val = (unsigned short)va_arg(list, int);
                break;
            case 'l':
                val = va_arg(list, unsigned long);
                break;
            case 'q':
                val = va_arg(list, unsigned long long);
                break;
            case 'j':
                val = va_arg(list, uintmax_t);
                break;
            case 'z':
                val = va_arg(list, size_t);
                break;
            default:
                val = va_arg(list, unsigned int);
                break;
            }

            if (specifier == 'd' || specifier == 'i')
                __int_str((intmax_t)val, intStrBuffer, base,
                          plusSign, spaceNoSign, width,
                          leftJustify, zeroPad);
            else
                __int_str(val, intStrBuffer, base,
                          plusSign, spaceNoSign, width,
                          leftJustify, zeroPad);

            out_string(out, ctx, intStrBuffer, &chars);
            continue;
        }

        /* char */

        if (specifier == 'c')
        {

            char c = (char)va_arg(list, int);
            out_char(out, ctx, c, &chars);
            continue;
        }

        /* string */

        if (specifier == 's')
        {
            const char *s = va_arg(list, const char *);
            if (!s)
                s = "(null)";

            int len = 0;
            while (s[len])
                len++;

            if (precision && precision < len)
                len = precision;

            for (int j = 0; j < len; j++)
                out_char(out, ctx, s[j], &chars);

            continue;
        }

        /* % */

        if (specifier == '%')
        {

            out_char(out, ctx, '%', &chars);
            continue;
        }
    }

    return chars;
}

void stdout_out(char c, void *ctx)
{
    (void)ctx;
    putc(c);
}

typedef struct
{
    char *buf;
    size_t pos;

} buffer_ctx;

void buffer_out(char c, void *ctx)
{
    buffer_ctx *b = ctx;
    b->buf[b->pos++] = c;
}

int vprintf(const char *format, va_list list)
{
    return vformat(stdout_out, NULL, format, list);
}

int printf(const char *format, ...)
{
    va_list list;

    va_start(list, format);
    int r = vformat(stdout_out, NULL, format, list);
    va_end(list);

    return r;
}

int vsprintf(char *str, const char *format, va_list list)
{
    buffer_ctx ctx = {str, 0};

    int r = vformat(buffer_out, &ctx, format, list);

    str[ctx.pos] = '\0';

    return r;
}

int sprintf(char *str, const char *format, ...)
{
    va_list list;

    va_start(list, format);
    int r = vsprintf(str, format, list);
    va_end(list);

    return r;
}