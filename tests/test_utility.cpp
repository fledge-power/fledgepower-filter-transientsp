#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sys/time.h>
#include <utility.h>

using namespace std;

TEST(TestUtility, convertTimestamp) 
{
    struct timeval timestamp;
    gettimeofday(&timestamp, NULL);
    long tsMs = timestamp.tv_sec * 1000;

    int testTime = 3000;

    for (int i = 0; i <= testTime ; i++ ) {
        long ts1 = tsMs + i ;
        std::pair<long, long> p = Utility::fromTimestamp(ts1);
        long ts2 = Utility::toTimestamp(p.first, p.second);

        ASSERT_EQ(ts1, ts2);
    }
}

