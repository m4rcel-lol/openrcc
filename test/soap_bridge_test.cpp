#include <gtest/gtest.h>

#include <string>

#include "soap/soap_bridge.hpp"

TEST(SoapBridgeTest, MapsOpenJobEnvelopeToProtobufRequest) {
    const std::string envelope =
        "<Envelope><Body><OpenJob><request_id>req-1</request_id><job_name>x</job_name><tick_rate_hz>30</tick_rate_hz></OpenJob></Body></Envelope>";

    const openrcc::v1::OpenJobRequest request = openrcc::soap::MapSoapToOpenJob(envelope);

    EXPECT_EQ(request.request_id(), "req-1");
    EXPECT_EQ(request.job_name(), "x");
    EXPECT_EQ(request.tick_rate_hz(), 30U);
}

TEST(SoapBridgeTest, SupportsNamespacePrefixesAndAttributes) {
    const std::string envelope =
        "<soap:Envelope><soap:Body><m:OpenJob><m:request_id id=\"1\">req-2</m:request_id>"
        "<m:job_name>prefixed</m:job_name><m:tick_rate_hz>60</m:tick_rate_hz></m:OpenJob></soap:Body></soap:Envelope>";

    const openrcc::v1::OpenJobRequest request = openrcc::soap::MapSoapToOpenJob(envelope);

    EXPECT_EQ(request.request_id(), "req-2");
    EXPECT_EQ(request.job_name(), "prefixed");
    EXPECT_EQ(request.tick_rate_hz(), 60U);
}

TEST(SoapBridgeTest, InvalidTickRateDoesNotThrow) {
    const std::string envelope =
        "<Envelope><Body><OpenJob><request_id>req-3</request_id><job_name>x</job_name>"
        "<tick_rate_hz>not-a-number</tick_rate_hz></OpenJob></Body></Envelope>";

    EXPECT_NO_THROW({
        const openrcc::v1::OpenJobRequest request = openrcc::soap::MapSoapToOpenJob(envelope);
        EXPECT_EQ(request.request_id(), "req-3");
        EXPECT_EQ(request.tick_rate_hz(), 0U);
    });
}
