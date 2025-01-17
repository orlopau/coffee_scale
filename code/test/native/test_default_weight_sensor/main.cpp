#include <unity.h>

#include "millis.h"
#include "button.h"
#include "weight_sensor.h"
#include "mock/mock_loadcell.h"

DefaultWeightSensor *weightSensor;

void setUp(void)
{
    weightSensor = new DefaultWeightSensor();
}

void tearDown(void)
{
    delete weightSensor;
}

void test_get_weight(void)
{
    LoadCell::value = 100;
    LoadCell::ready = true;
    weightSensor->update();
    TEST_ASSERT_EQUAL_FLOAT(100, weightSensor->getWeight());
}

void test_new_weight(void)
{
    LoadCell::value = 100;
    LoadCell::ready = true;
    weightSensor->update();
    LoadCell::ready = false;

    TEST_ASSERT_TRUE(weightSensor->isNewWeight());
    TEST_ASSERT_EQUAL_FLOAT(100, weightSensor->getWeight());

    weightSensor->update();
    TEST_ASSERT_FALSE(weightSensor->isNewWeight());
    TEST_ASSERT_EQUAL_FLOAT(100, weightSensor->getWeight());
}

void setWeight(long weight)
{
    LoadCell::value = weight;
    LoadCell::ready = true;
    weightSensor->update();
    LoadCell::ready = false;
}

void test_weight_averaging(void)
{
    // divide all weight changes by 10 and all sleeps by 10 to make the test run
    // faster but still resulting in the same delta weight change per second
    const static int DIVIDER = 10;

    weightSensor->setAutoAveraging(1000, 4);
    // when weight changes under 1000 per second, average over 4 samples

    // change weight faster than 1000 per second
    // start with 1000
    setWeight(1000 / DIVIDER);
    sleep_for(1000 / DIVIDER);
    setWeight(3000 / DIVIDER);

    // should return last weight
    TEST_ASSERT_EQUAL(3000 / DIVIDER, weightSensor->getRawWeight());

    // now increase slower than 1000 per second
    setWeight(4000 / DIVIDER);
    sleep_for(1000 / DIVIDER);
    setWeight(4500 / DIVIDER);

    // now averaging should be enabled, but it only returns the average of values
    // that have been read after averaging was enabled, i.e. now it should return
    // only the last value.
    TEST_ASSERT_EQUAL(4500 / DIVIDER, weightSensor->getRawWeight());

    // add some more values slowly
    sleep_for(1000 / DIVIDER);
    setWeight(5000 / DIVIDER);
    // average of last 2 samples
    TEST_ASSERT_EQUAL(4750 / DIVIDER, weightSensor->getRawWeight());
    sleep_for(1000 / DIVIDER);
    setWeight(5500 / DIVIDER);
    // average of last 3 samples
    TEST_ASSERT_EQUAL(5000 / DIVIDER, weightSensor->getRawWeight());
    sleep_for(1000 / DIVIDER);
    setWeight(6000 / DIVIDER);

    // should return averaged weight over last 4 samples
    TEST_ASSERT_EQUAL(5250 / DIVIDER, weightSensor->getRawWeight());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_get_weight);
    RUN_TEST(test_new_weight);
    RUN_TEST(test_weight_averaging);
    UNITY_END();
}