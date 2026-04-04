#pragma once

namespace openrcc::tutorial {

/**
 * Execute the self-guided OpenRCC tutorial simulation.
 *
 * @param non_interactive True to skip pause prompts.
 * @return Process exit code.
 */
int RunTutorial(bool non_interactive);

}  // namespace openrcc::tutorial
