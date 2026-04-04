#include "tutorial/tutorial_mode.hpp"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace openrcc::tutorial {

/**
 * Pause for Enter key in interactive mode.
 *
 * @param non_interactive True when pauses should be skipped.
 */
static void Pause(bool non_interactive) {
    if (non_interactive) {
        return;
    }
    std::cout << "\nPress Enter to continue...";
    std::string line;
    std::getline(std::cin, line);
}

/**
 * Print a numbered tutorial step with ANSI color.
 *
 * @param index Step number.
 * @param content Step content.
 */
static void PrintStep(int index, const std::string& content) {
    std::cout << "\033[1;36m[Step " << index << "]\033[0m " << content << "\n";
}

int RunTutorial(bool non_interactive) {
    // ARCH: Tutorial uses deterministic scripted output to teach architecture without requiring a live network port.
    PrintStep(1, "Service startup and simulated systemd READY=1 notification.");
    Pause(non_interactive);

    PrintStep(2, "Receiving OpenJob gRPC call. Raw protobuf bytes: 0a 24 72 65 71 75 65 73 74 ...");
    std::cout << "Decoded: request_id=\"request-0001\", job_name=\"tutorial-job\", tick_rate_hz=30\n";
    Pause(non_interactive);

    PrintStep(3, "Job FSM transition: PENDING -> RUNNING");
    Pause(non_interactive);

    PrintStep(4, "Luau VM initialization and sandbox capability deny-list application.");
    Pause(non_interactive);

    PrintStep(5, "Receiving ExecuteScript. Luau source:\n  rcc.log('hello from tutorial')\n");
    std::cout << "Compiled bytecode chunk: [simulated educational output]\n";
    Pause(non_interactive);

    PrintStep(6, "Tick loop and instruction budget enforcement via VM interrupt callbacks.");
    Pause(non_interactive);

    PrintStep(7, "CloseJob request, FSM drain, and teardown complete.");
    return 0;
}

}  // namespace openrcc::tutorial
