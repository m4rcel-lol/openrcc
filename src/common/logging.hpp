#pragma once

namespace openrcc::common {

/**
 * Initialize global logging sinks.
 *
 * @param json_mode True for structured JSON logs, false for human-readable logs.
 */
void InitLogging(bool json_mode);

}  // namespace openrcc::common
