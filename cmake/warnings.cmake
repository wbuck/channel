function(set_project_warnings proj_name)
    set(GCC_WARNINGS
        -Wall
        -Wextra
        -Wpedantic               # Warn if non-standard C++ is used.
        -Wshadow                 # Warn the user if a variable declaration shadows one from a parent context.
        -Wcast-align             # Warn for potential performance problem casts.
        -Wunused                 # Warn on anything being unused.
        -Wconversion             # Warn on type conversions that may lose data.
        -Wsign-conversion        # Warn on sign conversions.
        -Wnull-dereference       # Warn if a null dereference is detected.
        -Wdouble-promotion       # Warn if float is implicit promoted to double.
        -Wformat=2               # Warn on security issues around functions that format output (ie printf).
        -Wmisleading-indentation # Warn if indentation implies blocks where blocks do not exist.
        -Wduplicated-cond        # Warn if if / else chain has duplicated conditions.
        -Wduplicated-branches    # Warn if if / else branches have duplicated code.
        -Wlogical-op             # Warn about logical operations being used where bitwise were probably wanted.
        -Wmisleading-indentation # Warn if indentation implies blocks where blocks do not exist
        -Wduplicated-cond        # Warn if if / else chain has duplicated conditions
        -Wduplicated-branches    # Warn if if / else branches have duplicated code
        -Wlogical-op             # Warn about logical operations being used where bitwise were probably wanted
        -Wuseless-cast           # Warn if you perform a cast to the same type
        -Woverloaded-virtual     # Warn if you overload (not override) a virtual function
        -Wold-style-cast         # Warn for c-style casts
        -Wnon-virtual-dtor       # Warn the user if a class with virtual functions has a non-virtual destructor.
        #-Wpadded                # Warn for inefficient struct layout.
    )

    set(PROJECT_WARNINGS ${GCC_WARNINGS})
    target_compile_options(${proj_name} INTERFACE ${PROJECT_WARNINGS})

endfunction()