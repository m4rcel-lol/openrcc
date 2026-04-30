#pragma once

#include <string>

#include "rcc_control.pb.h"

namespace openrcc::soap {

/**
 * Extract a simple XML tag value.
 *
 * Supports exact tag names, namespace-prefixed tag names, and attributes on the
 * opening tag. This helper is intentionally small and is not a general XML parser.
 *
 * @param xml XML payload.
 * @param tag Tag name without namespace prefix.
 * @return Extracted value or empty string.
 */
std::string ExtractTagValue(const std::string& xml, const std::string& tag);

/**
 * Translate SOAP/XML OpenJob envelope to protobuf request.
 *
 * Invalid or missing numeric fields are left at protobuf defaults instead of
 * throwing, so malformed SOAP ingress cannot crash the bridge process.
 *
 * @param xml SOAP XML string.
 * @return OpenJob protobuf request.
 */
openrcc::v1::OpenJobRequest MapSoapToOpenJob(const std::string& xml);

}  // namespace openrcc::soap
