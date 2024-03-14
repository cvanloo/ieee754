#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define BITS(b, i, n) ((b) >> (i)) & ((1 << (n)) - 1)

#define SIGN(b) BITS(b, 31, 1)
#define EXP(b) BITS(b, 23, 8)
#define MANT(b) BITS(b, 0, 23)

//typedef uint32_t Float;

typedef union Float Float;
union Float {
    uint32_t i;
    struct {
        //_BitInt (1) sign;
        unsigned int sign :1;
        _BitInt (8) exp;
        _BitInt (23) mant;
    } f;
};

typedef union Float_Conv Float_Conv;
union Float_Conv {
    float f;
    Float i;
};

void print_float_bits(Float a);
Float addition(Float a, Float b);

int main(void) {
    Float_Conv fb_a, fb_b;
    Float a, b;
    //fb_a.f = -4.2f;
    //fb_b.f = 5.6f;
    fb_a.f = 1.5f;
    fb_b.f = 0.75f;
    a = fb_a.i;
    b = fb_b.i;

    printf("[S][Exponent][Mantissa...............]\n");
    printf("a: "); print_float_bits(a);
    printf("b: "); print_float_bits(b);

    //printf("%#08x\n", a);
    //printf("%#08x\n", b);
    Float r = addition(a, b);
    printf("r: "); print_float_bits(r);
    //printf("%#08x\n", r);
    return EXIT_SUCCESS;
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

uint32_t shift_mantissa(uint32_t m, uint8_t n) {
    m = m << 9;
    m = ~(~m >> 1);
    m = m >> n;
    m = m >> 9;
    return m;
}

Float addition(Float a, Float b) {
    uint8_t sign_a = SIGN(a);
    uint8_t sign_b = SIGN(b);
    uint8_t exp_a = EXP(a);
    uint8_t exp_b = EXP(b);
    uint32_t mant_a = MANT(a);
    uint32_t mant_b = MANT(b);

    if (sign_a != sign_b) {
        // @todo: do subtraction
    }

    // @fixme: a needs to be larger than b (?)
    assert(exp_a >= exp_b);

    mant_a = shift_mantissa(mant_a, 0);
    mant_b = shift_mantissa(mant_b, exp_a - exp_b);

    uint32_t mantissa_r = mant_a + mant_b;
    print_float_bits(mantissa_r);

    return (Float) 0;
}
