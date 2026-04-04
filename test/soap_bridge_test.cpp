#include <gtest/gtest.h>

#include <string>

/**
 * Minimal educational placeholder SOAP translation test.
 */
TEST(SoapBridgeTest, RawEnvelopeContainsOpenJobRequestShape) {
    const std::string envelope =
        "<Envelope><Body><OpenJob><request_id>req-1</request_id><job_name>x</job_name><tick_rate_hz>30</tick_rate_hz></OpenJob></Body></Envelope>";
    EXPECT_NE(envelope.find("<OpenJob>"), std::string::npos);
    EXPECT_NE(envelope.find("<request_id>"), std::string::npos);
}
