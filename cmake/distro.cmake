function(openrcc_print_distro_summary)
    execute_process(
        COMMAND uname -r
        OUTPUT_VARIABLE OPENRCC_KERNEL_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE OPENRCC_MACHINE_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT OPENRCC_KERNEL_VERSION)
        set(OPENRCC_KERNEL_VERSION "unknown")
    endif()
    if(NOT OPENRCC_MACHINE_ARCH)
        set(OPENRCC_MACHINE_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    set(OPENRCC_OS_ID "unknown")
    set(OPENRCC_OS_LIKE "unknown")
    set(OPENRCC_PACKAGE_MANAGER "unknown")

    if(APPLE)
        execute_process(
            COMMAND sw_vers -productVersion
            OUTPUT_VARIABLE OPENRCC_MACOS_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        set(OPENRCC_OS_ID "macos")
        set(OPENRCC_OS_LIKE "darwin")
        if(OPENRCC_MACOS_VERSION)
            set(OPENRCC_OS_ID "macos-${OPENRCC_MACOS_VERSION}")
        endif()

        find_program(OPENRCC_HOMEBREW brew PATHS /opt/homebrew/bin /usr/local/bin)
        if(OPENRCC_HOMEBREW)
            set(OPENRCC_PACKAGE_MANAGER "homebrew")
        endif()
    elseif(EXISTS "/etc/os-release")
        file(STRINGS "/etc/os-release" OPENRCC_OS_RELEASE_LINES)
        foreach(OPENRCC_LINE IN LISTS OPENRCC_OS_RELEASE_LINES)
            if(OPENRCC_LINE MATCHES "^ID=")
                string(REPLACE "ID=" "" OPENRCC_OS_ID "${OPENRCC_LINE}")
                string(REPLACE "\"" "" OPENRCC_OS_ID "${OPENRCC_OS_ID}")
            endif()
            if(OPENRCC_LINE MATCHES "^ID_LIKE=")
                string(REPLACE "ID_LIKE=" "" OPENRCC_OS_LIKE "${OPENRCC_LINE}")
                string(REPLACE "\"" "" OPENRCC_OS_LIKE "${OPENRCC_OS_LIKE}")
            endif()
        endforeach()
    endif()

    if(OPENRCC_OS_ID MATCHES "(arch|cachyos|manjaro|endeavouros)" OR OPENRCC_OS_LIKE MATCHES "arch")
        set(OPENRCC_PACKAGE_MANAGER "pacman")
    elseif(OPENRCC_OS_ID MATCHES "(fedora|bazzite|nobara|silverblue|kinoite)" OR OPENRCC_OS_LIKE MATCHES "fedora")
        if(EXISTS "/usr/bin/rpm-ostree")
            set(OPENRCC_PACKAGE_MANAGER "rpm-ostree")
        else()
            set(OPENRCC_PACKAGE_MANAGER "dnf")
        endif()
    endif()

    message(STATUS "[OpenRCC] Distro detection summary")
    message(STATUS "[OpenRCC]   ID: ${OPENRCC_OS_ID}")
    message(STATUS "[OpenRCC]   ID_LIKE: ${OPENRCC_OS_LIKE}")
    message(STATUS "[OpenRCC]   Package manager: ${OPENRCC_PACKAGE_MANAGER}")
    message(STATUS "[OpenRCC]   Kernel version: ${OPENRCC_KERNEL_VERSION}")
    message(STATUS "[OpenRCC]   Architecture: ${OPENRCC_MACHINE_ARCH}")
endfunction()
