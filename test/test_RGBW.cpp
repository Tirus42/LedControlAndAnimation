#include <unity.h>
#include "RGBW.h"

static void assert_rgbw_equal(const RGBW& a, const RGBW& b) {
    TEST_ASSERT_TRUE(a == b);
}

static void test_default_constructor() {
    RGBW c;

    TEST_ASSERT_EQUAL(0u, c.r);
    TEST_ASSERT_EQUAL(0u, c.g);
    TEST_ASSERT_EQUAL(0u, c.b);
    TEST_ASSERT_EQUAL(0u, c.w);
}

static void test_component_constructor() {
    RGBW c(12, 34, 56, 78);
    TEST_ASSERT_EQUAL(12u, c.r);
    TEST_ASSERT_EQUAL(34u, c.g);
    TEST_ASSERT_EQUAL(56u, c.b);
    TEST_ASSERT_EQUAL(78u, c.w);
}

static void test_packed_constructor() {
    /* WRGB order: w=0xAA, r=0x11, g=0x22, b=0x33 */
    const uint32_t packed = (0xAA << 24) | (0x11 << 16)
    | (0x22 << 8 ) | 0x33;
    RGBW c(packed);
    TEST_ASSERT_EQUAL(0xAAu, c.w);
    TEST_ASSERT_EQUAL(0x11u, c.r);
    TEST_ASSERT_EQUAL(0x22u, c.g);
    TEST_ASSERT_EQUAL(0x33u, c.b);
}

static void test_get_as_packed_color() {
    RGBW c(1,2,3,4);          /* R=1 G=2 B=3 W=4 */
    uint32_t packed = c.getAsPackedColor();
    TEST_ASSERT_EQUAL((uint32_t(4) << 24 | 1 << 16 | 2 << 8 | 3), packed);
}

static void test_addition_operator() {
    RGBW a{10,20,30,40};
    RGBW b{5,5,5,5};
    RGBW c = a + b;
    TEST_ASSERT_EQUAL(15u, c.r);
    TEST_ASSERT_EQUAL(25u, c.g);
    TEST_ASSERT_EQUAL(35u, c.b);
    TEST_ASSERT_EQUAL(45u, c.w);
}

static void test_addition_operator_overflow() {
    RGBW a{250, 251, 100, 0};
    RGBW b{5, 5, 5, 5};
    RGBW c = a + b;

    TEST_ASSERT_EQUAL(255, c.r);
    TEST_ASSERT_EQUAL(255, c.g);    // No overflow here
    TEST_ASSERT_EQUAL(105, c.b);
    TEST_ASSERT_EQUAL(5, c.w);
}

static void test_scalar_multiplication() {
    RGBW a{10,20,30,40};
    RGBW c = a * 1.5f;
    /* The original code truncates toward zero; Unity’s assert
     *      will still pass because we cast to uint8_t inside operator*. */
    TEST_ASSERT_EQUAL(15u, c.r);
    TEST_ASSERT_EQUAL(30u, c.g);
    TEST_ASSERT_EQUAL(45u, c.b);
    TEST_ASSERT_EQUAL(60u, c.w);
}

static void test_interpolation() {
    RGBW a{0,0,0,0};
    RGBW b{100,200,255,50};

    /* factor = 0 → a */
    RGBW c0 = a.interpolateTo(b, 0.0f);
    assert_rgbw_equal(a, c0);

    /* factor = 1 → b */
    RGBW c1 = a.interpolateTo(b, 1.0f);
    assert_rgbw_equal(b, c1);

    /* factor = 0.5 → halfway (truncated) */
    RGBW half = a.interpolateTo(b, 0.5f);
    TEST_ASSERT_EQUAL(50u, half.r);   // (0+100)/2
    TEST_ASSERT_EQUAL(100u, half.g);  // (0+200)/2
    TEST_ASSERT_EQUAL(127u, half.b);  // (0+255)/2 → 127
    TEST_ASSERT_EQUAL(25u, half.w);
}

void test_total_brightness() {
    RGBW c{10,20,30,40};
    TEST_ASSERT_EQUAL(100u, c.getTotalBrightness());
}

void test_with_total_brightness() {
    RGBW c{200,150,100,50};          /* sum = 500 */
    RGBW d = c.getWithTotalBrightness(300);   /* target < current */

    TEST_ASSERT_EQUAL(300, d.getTotalBrightness());
    /* All components should not exceed the originals. */
    TEST_ASSERT_TRUE(d.r <= c.r && d.g <= c.g &&
    d.b <= c.b && d.w <= c.w);
}

void test_with_total_brightness_small_values() {
    RGBW c{200,150,100,50};

    RGBW b_100 = c.getWithTotalBrightness(100);
    RGBW b_10 = c.getWithTotalBrightness(10);
    RGBW b_1 = c.getWithTotalBrightness(1);
    RGBW b_0 = c.getWithTotalBrightness(0);

    TEST_ASSERT_LESS_OR_EQUAL(100, b_100.getTotalBrightness());
    TEST_ASSERT_LESS_OR_EQUAL(10, b_10.getTotalBrightness());
    TEST_ASSERT_LESS_OR_EQUAL(1, b_1.getTotalBrightness());
    TEST_ASSERT_EQUAL(0, b_0.getTotalBrightness());
    assert_rgbw_equal(RGBW(), b_0);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_default_constructor);
    RUN_TEST(test_component_constructor);
    RUN_TEST(test_packed_constructor);
    RUN_TEST(test_get_as_packed_color);
    RUN_TEST(test_addition_operator);
    RUN_TEST(test_addition_operator_overflow);
    RUN_TEST(test_scalar_multiplication);
    RUN_TEST(test_interpolation);
    RUN_TEST(test_total_brightness);
    RUN_TEST(test_with_total_brightness);
    RUN_TEST(test_with_total_brightness_small_values);
    return UNITY_END();
}
