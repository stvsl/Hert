# file-collector.cmake

# 用法:
# file_collector(<out_var> FOLDERS <folder1> [<folder2> ...] EXTENSIONS <ext1> [<ext2> ...])
#
# 例:
# file_collector(MY_FILES FOLDERS ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include EXTENSIONS .cpp .hpp)

function(file_collector out_var)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs FOLDERS EXTENSIONS)
    cmake_parse_arguments(FC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT FC_FOLDERS OR NOT FC_EXTENSIONS)
        message(FATAL_ERROR "file_collector: FOLDERS 和 EXTENSIONS 参数不能为空")
    endif()

    set(result)
    foreach(folder IN LISTS FC_FOLDERS)
        foreach(ext IN LISTS FC_EXTENSIONS)
            file(GLOB_RECURSE found_files
                "${folder}/*${ext}"
            )
            list(APPEND result ${found_files})
        endforeach()
    endforeach()
    list(REMOVE_DUPLICATES result)
    set(${out_var} ${result} PARENT_SCOPE)
    if(NOT result)
        message(FATAL_ERROR "file_collector: 在指定的文件夹中未找到任何匹配的文件")
    endif()
    # 输出收集到的文件列表到build/file_collect/<out_var>.txt
    file(WRITE "${CMAKE_BINARY_DIR}/file_collect/${out_var}.txt" "")
    foreach(file IN LISTS result)
        file(APPEND "${CMAKE_BINARY_DIR}/file_collect/${out_var}.txt" "${file}\n")
    endforeach()
    message(STATUS "file_collector: 文件列表已写入 ${CMAKE_BINARY_DIR}/file_collect/${out_var}.txt")
    set(${out_var} ${result} PARENT_SCOPE)
    return()
endfunction()
