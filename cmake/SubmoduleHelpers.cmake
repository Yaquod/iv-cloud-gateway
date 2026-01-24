# This file contains helper functions for managing submodules.

function(vgw_add_subdirectory_no_install target)
    # Temporarily override the install command to do nothing.
    macro(install)
        # Do nothing
    endmacro()

    # Also override the export() command, which is used for packaging.
    macro(export)
        # Do nothing
    endmacro()

    add_subdirectory(${target} ${ARGN})

    # Restore the original install command.
    macro(install)
        _install(${ARGV})
    endmacro()

    # Restore the original export command.
    macro(export)
        _export(${ARGV})
    endmacro()

    # Ensure the temporary macros do not leak to the parent scope.
    unset(install PARENT_SCOPE)
    unset(export PARENT_SCOPE)
endfunction()