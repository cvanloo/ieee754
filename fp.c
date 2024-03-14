#include <stdio.h>
#include <stdint.h>

#define BITS(b, i, n) ((b) >> (i)) & ((1 << (n)) - 1)

#define SIGN(b) BITS(b, 31, 1)
#define EXP(b) BITS(b, 23, 8)
#define MANT(b) BITS(b, 0, 23)

static int failed_tests = 0;
#define TEST(a, b) do { \
    int ok = test_two_floats(a, b); \
    if (!ok) { \
        ++failed_tests; \
    } \
    printf("testing " #a " + " #b ": " "%s" "\n", ok ? "OK" : "FAILED"); \
} while(0)

typedef uint32_t Float;

typedef union Float_Conv Float_Conv;
union Float_Conv {
    float f;
    Float i;
};

void print_float_bits(Float f);
uint32_t prepend_hidden_bit(uint32_t mantissa, uint8_t exponent);
uint32_t shift_mantissa(uint32_t mantissa, int8_t shift_amount);
int highest_one_bit(uint32_t bits);
void normalize_mantissa(uint32_t *mantissa, uint8_t *exponent);
Float bits_to_float(uint8_t sign, uint8_t exponent, uint32_t mantissa);
Float addition(Float a, Float b);
int test_two_floats(float a, float b);

int main(void) {
    TEST(1.5f, 0.75f); // example from lecture

    TEST(5.1f, 2.8f); // first mantissa smaller than second, first exponent bigger than second
    TEST(2.8f, 5.1f); // first mantissa bigger than second, first exponent smaller than second

    TEST(1.5f, 3.25f); // first mantissa and exponent smaller than second
    TEST(3.25f, 1.5f); // first mantissa and exponent bigger than second

    TEST(-5.1f, 2.8f); // first mantissa smaller than second, first exponent bigger than second
    TEST(2.8f, -5.1f); // first mantissa bigger than second, first exponent smaller than second

    TEST(1.5f, -3.25f); // first mantissa and exponent smaller than second
    TEST(-3.25f, 1.5f); // first mantissa and exponent bigger than second

    TEST(-5.1f, -2.8f); // both negative: first mantissa smaller than second, first exponent bigger than second
    TEST(-2.8f, -5.1f); // both negative: first mantissa bigger than second, first exponent smaller than second

    TEST(-1.5f, -3.25f); // both negative: first mantissa bigger than second, first exponent smaller than second
    TEST(-3.25f, -1.5f); // both negative: first mantissa smaller than second, first exponent bigger than second

    float zero = 0.0f;
    float value = 419037.1875f;
    float nonZeroMantissa1 = 5.90381056005e-41f;
    float nonZeroMantissa2 = 7.35996705665e-39f;

    TEST(zero, value);
    TEST(value, zero);
    TEST(zero, zero);
    TEST(nonZeroMantissa1, nonZeroMantissa2);
    TEST(nonZeroMantissa2, zero);

    return failed_tests;
}

void print_float_bits(Float a) {
    printf("[");
    for (int i = (sizeof a)*8-1; i >= 0; --i) {
        if (i == 22 || i == 30) {
            printf("][");
        }
        uint8_t byte = BITS(a, i, 1);
        printf("%u", byte);
    }
    printf("]\n");
}

uint32_t prepend_hidden_bit(uint32_t m, uint8_t e) {
    return e == 0
        ? m
        : m | (1 << 23);
}

uint32_t shift_mantissa(uint32_t m, int8_t n) {
    return n > 0
        ? m >> n
        : m << (-1*n);
}

int highest_one_bit(uint32_t b) {
    if (b == 0) return -1;
    int i = 0;
    while (b > 1) {
        ++i;
        b = b >> 1;
    }
    return i;
}

void normalize_mantissa(uint32_t *mant, uint8_t *exp) {
    if (*exp == 0) return;
    int hb = highest_one_bit(*mant);
    if (hb == -1) return;
    int shift = hb - 23;
    *mant = shift_mantissa(*mant, shift);
    *exp = *exp + shift;
}

Float bits_to_float(uint8_t sign, uint8_t exp, uint32_t mant) {
    Float r = sign << 31;
    r = r | (exp << 23);
    r = r | (mant & 0x7FFFFF);
    return r;
}

// @note: https://www.rfwireless-world.com/Tutorials/floating-point-tutorial.html
Float addition(Float a, Float b) {
    uint32_t abs_a = a & 0x7FFFFFFF;
    uint32_t abs_b = b & 0x7FFFFFFF;

    if (abs_a < abs_b) {
        return addition(b, a);
    }

    uint8_t sign_a = SIGN(a);
    uint8_t sign_b = SIGN(b);
    uint8_t exp_a = EXP(a);
    uint8_t exp_b = EXP(b);
    uint32_t mant_a = MANT(a);
    uint32_t mant_b = MANT(b);

    mant_a = prepend_hidden_bit(mant_a, exp_a);
    mant_b = prepend_hidden_bit(mant_b, exp_b);

    uint8_t exp_a_cmp = exp_a == 0 ? 1 : exp_a;
    uint8_t exp_b_cmp = exp_b == 0 ? 1 : exp_b;

    uint8_t exp_r = exp_a;

    if (exp_a_cmp - exp_b_cmp != 0) {
        if (mant_a > mant_b) {
            mant_b = shift_mantissa(mant_b, exp_a_cmp - exp_b_cmp);
            // exp_r = exp_a;
        } else {
            mant_a = shift_mantissa(mant_a, exp_b_cmp - exp_a_cmp);
            exp_r = exp_b;
        }
    }

    uint32_t mant_r;
    if (sign_a == sign_b) {
        mant_r = mant_a + mant_b;
    } else {
        mant_r = mant_a - mant_b;
    }

    normalize_mantissa(&mant_r, &exp_r);

    Float r = bits_to_float(sign_a, exp_r, mant_r);
    return r;
}

int test_two_floats(float a, float b) {
    float expected = a + b;
    Float_Conv ca, cb, cr, ce;
    ca.f = a;
    cb.f = b;
    Float result = addition(ca.i, cb.i);
    cr.i = result;
    ce.f = expected;
    //printf("expected: "); print_float_bits(ce.i);
    //printf("  actual: "); print_float_bits(cr.i);
    return ce.i == cr.i; // compare bits and not float value
}
