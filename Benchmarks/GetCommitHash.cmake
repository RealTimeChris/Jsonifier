# Define a function to get the short commit hash
function(getCommitHash repo_path commit_var)
    execute_process(
        COMMAND git -C ${repo_path} rev-parse --short HEAD
        OUTPUT_VARIABLE git_commit
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE git_result
    )
    if (git_result EQUAL 0)
        set(${commit_var} "${git_commit}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Failed to get the short commit hash from ${repo_path}")
    endif()
endfunction()