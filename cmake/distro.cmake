function(openrcc_print_distro_summary)
    execute_process(
        COMMAND uname -r
        OUTPUT_VARIABLE OPENRCC_KERNEL_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(OPENRCC_OS_ID "unknown")
    set(OPENRCC_OS_LIKE "unknown")

    if(EXISTS "/etc/os-release")
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

    set(OPENRCC_PACKAGE_MANAGER "unknown")
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
endfunction()
