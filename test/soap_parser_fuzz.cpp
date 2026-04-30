#include <cstddef>
#include <cstdint>
#include <string>

#include "soap/soap_bridge.hpp"

/**
 * Fuzz target for SOAP XML parser ingress.
 *
 * @param data Arbitrary input bytes.
 * @param size Number of bytes.
 * @return 0 always.
 */
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::string payload(reinterpret_cast<const char*>(data), size);
    (void)openrcc::soap::MapSoapToOpenJob(payload);
    return 0;
}
