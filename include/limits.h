#ifndef _LIMITS_H_
#define _LIMITS_H_

/* Number of bits in a `char'. */
#define CHAR_BIT 8

/* Minimum and maximum values a `signed char' can hold. */
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127

/* Maximum value an `unsigned char' can hold. */
#define UCHAR_MAX 255

/* Minimum and maximum values a `char' can hold. */
#define CHAR_MIN (-128)
#define CHAR_MAX 127

/* Minimum and maximum values a `signed short int' can hold. */
#define SHRT_MIN (-32768)
#define SHRT_MAX 32767

/* Maximum value an `unsigned short int' can hold. */
#define USHRT_MAX 65535

/* Minimum and maximum values a `signed int' can hold. */
#define INT_MIN (-2147483648)
#define INT_MAX 2147483647

/* Maximum value an `unsigned int' can hold. */
#define UINT_MAX (INT_MAX * 2U + 1)

/* Minimum and maximum values a `signed long int' can hold. */
#define LONG_MIN (-2147483648L)
#define LONG_MAX 2147483647L

/* Maximum value an `unsigned long int' can hold. */
#define ULONG_MAX (LONG_MAX * 2UL + 1)

/* Minimum and maximum values a `signed long long int' can hold.  */
#define LONG_LONG_MIN (-9223372036854775808LL)
#define LONG_LONG_MAX 9223372036854775807LL

/* Maximum value an `unsigned long long int' can hold. */
#define ULONG_LONG_MAX (LONG_LONG_MAX * 2ULL + 1)

#endif /* !_LIMITS_H_ */
