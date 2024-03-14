#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define BITS(b, i, n) ((b) >> (i)) & ((1 << (n)) - 1)

#define SIGN(b) BITS(b, 31, 1)
#define EXP(b) BITS(b, 23, 8)
#define MANT(b) BITS(b, 0, 23)

#define TEST(a, b) printf("testing " #a " + " #b ": " "%s" "\n", test_two_floats(a, b) ? "OK" : "FAILED");

typedef uint32_t Float;

//typedef union Float Float;
//union Float {
//    uint32_t i;
//    struct {
//        unsigned int sign :1;
//        unsigned int exp :8;
//        unsigned int mant :23;
//    } f;
//};

typedef union Float_Conv Float_Conv;
union Float_Conv {
    float f;
    Float i;
};

void print_float_bits(Float a);
uint32_t shift_mantissa(uint32_t m, uint8_t n);
int highest_one_bit(uint32_t b);
Float bits_to_float(uint8_t sign, uint8_t exp, uint32_t mant);
Float addition(Float a, Float b);
bool test_two_floats(float a, float b);

int main(void) {
    TEST(5.1f, 2.8f);
    TEST(2.8f, 5.1f);

    TEST(1.5f, 3.25f);
    TEST(3.25f, 1.5f);

    TEST(-5.1f, 2.8f);
    TEST(2.8f, -5.1f);

    TEST(1.5f, -3.25f);
    TEST(-3.25f, 1.5f);

    TEST(-5.1f, -2.8f);
    TEST(-2.8f, -5.1f);

    TEST(-1.5f, -3.25f);
    TEST(-3.25f, -1.5f);

    float zero = 0.0f;
    float value = 419037.1875f;
    float nonZeroMantissa1 = 5.90381056005e-41f;
    float nonZeroMantissa2 = 7.35996705665e-39f;

    TEST(zero, value);
    TEST(value, zero);
    TEST(zero, zero);
    TEST(nonZeroMantissa1, nonZeroMantissa2);
    TEST(nonZeroMantissa2, zero);

    return 0;
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

// n=0 will only shift in the hidden bit
uint32_t shift_mantissa(uint32_t m, uint8_t n) {
    m = m << 9;
    m = ~(~m >> 1);
    m = m >> 8;
    m = m >> n;
    return m;
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

Float bits_to_float(uint8_t sign, uint8_t exp, uint32_t mant) {
    Float r = sign << 31;
    r = r | (exp << 23);
    r = r | (mant & 0x7FFFFF);
    return r;
}

Float addition(Float a, Float b) {
    uint8_t exp_a = EXP(a);
    uint8_t exp_b = EXP(b);

    if (exp_a < exp_b) return addition(b, a);

    uint8_t sign_a = SIGN(a);
    uint8_t sign_b = SIGN(b);
    uint32_t mant_a = MANT(a);
    uint32_t mant_b = MANT(b);

    uint8_t sign_r = sign_a;
    uint8_t exp_r = exp_a;
    uint32_t mant_r;

    mant_a = exp_a == 0 ? mant_a : shift_mantissa(mant_a, 0);
    mant_b = exp_b == 0 ? mant_b : shift_mantissa(mant_b, exp_a - exp_b);

    //printf("~a "); print_float_bits(mant_a);
    //printf("~b "); print_float_bits(mant_b);

    if (sign_a == sign_b) {
        mant_r = mant_a + mant_b;
    } else {
        mant_r = mant_a - mant_b;
    }

    //printf("+/- "); print_float_bits(mant_r);
 
    int hb = highest_one_bit(mant_r);
    if (hb != -1) {
        int shift = hb - 23;
        if (shift > 0) {
            mant_r = mant_r >> shift;
        } else if (shift < 0) {
            mant_r = mant_r << (-1*shift);
        }

        //printf("hb %d\n", hb);
        //printf("norm "); print_float_bits(mant_r);

        exp_r = exp_a + shift;
    } else {
        printf("here");
        exp_r = 127;
    }

    //printf("er %d\n", exp_r);

    return bits_to_float(sign_r, exp_r, mant_r);
}

bool test_two_floats(float a, float b) {
    float expected = a + b;
    Float_Conv ca, cb, cr;
    ca.f = a;
    cb.f = b;
    Float result = addition(ca.i, cb.i);
    cr.i = result;
    return expected == cr.f;
}
