#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/fff.h>

#include <cc_lib/aldrin.h>

#include <zephyr/drivers/pwm/pwm_fake.h>

DEFINE_FFF_GLOBALS;

static void before_test_setup(void* f)
{
    aldrin_init();
}

ZTEST_SUITE(aldrin_tests, NULL, NULL, before_test_setup, NULL, NULL);

ZTEST(aldrin_tests, test__buzz_short)
{
    zassert_equal(fake_pwm_set_cycles_fake.call_count, 0);
    aldrin_buzz_short();

    zassert_equal(fake_pwm_set_cycles_fake.call_count, 1,
                  "PWM should be called once, but was %d!",
                  fake_pwm_set_cycles_fake.call_count);
    zassert_not_equal(fake_pwm_set_cycles_fake.arg2_val, 0,
                      "Period cycles should not be 0, but is!");
    zassert_not_equal(fake_pwm_set_cycles_fake.arg3_val, 0,
                      "Pulse cycles should not be 0, but is!");

    // give stop timer some time to fire.
    k_sleep(K_MSEC(300));

    zassert_equal(fake_pwm_set_cycles_fake.call_count, 2,
                  "PWM should be called twice now, but was %d!",
                  fake_pwm_set_cycles_fake.call_count);
    zassert_equal(fake_pwm_set_cycles_fake.arg3_val, 0,
                  "Pulse cycles should be 0 now, but was %d!",
                  fake_pwm_set_cycles_fake.arg3_val);
}