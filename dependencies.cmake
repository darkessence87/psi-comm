
function (find_submodule name path)
    if (EXISTS ../../${name})
        set (${path} ../../${name} PARENT_SCOPE)
    elseif (EXISTS ${3rdPARTY_DIR}/${name})
        set (${path} ${3rdPARTY_DIR}/${name} PARENT_SCOPE)
    endif()
endfunction()

if (EXISTS 3rdparty/psi-tools)
    add_subdirectory(3rdparty/psi-tools)
endif()