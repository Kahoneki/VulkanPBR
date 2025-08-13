cmake_minimum_required(VERSION 3.28)
project(VulkanPBR)
set(CMAKE_CXX_STANDARD 23)
set(PROJECT_OUTPUT_DIR "${CMAKE_BINARY_DIR}")

#Find all .cpp and .h files recursively
file(GLOB_RECURSE ALL_PROJECT_FILES
    "${CMAKE_SOURCE_DIR}/src/*.h" 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

#Output all the source files to check glob's success
message(STATUS "---- Source Files ---- dir: ${PROJECT_OUTPUT_DIR}")
foreach(FILE ${ALL_PROJECT_FILES})
    message(STATUS " ${FILE}")
endforeach()
message(STATUS "--------------------------------")

#Create the executable
add_executable(VulkanPBR ${ALL_PROJECT_FILES})

#Add NekiVK
include(FetchContent)
FetchContent_Declare(
        NekiVK
        GIT_REPOSITORY "https://github.com/Kahoneki/NekiVK.git"
        GIT_TAG "v2.0.2"
)
FetchContent_MakeAvailable(NekiVK)
target_link_libraries(VulkanPBR PRIVATE NekiVK)

#Shader compilation
find_program(
        GLSLC_EXECUTABLE
        NAMES glslc
        HINTS "$ENV{VULKAN_SDK}/bin"
        REQUIRED
        DOC "Path to the GLSL to SPIR-V compiler"
)
message(STATUS "Found glslc: ${GLSLC_EXECUTABLE}")

#Define shader files
file(GLOB_RECURSE SHADER_SOURCES
    "${CMAKE_SOURCE_DIR}/Shaders/*.vert"
    "${CMAKE_SOURCE_DIR}/Shaders/*.frag"
)
message(STATUS "---- Found Shader Files ----")
foreach(FILE ${SHADER_SOURCES})
    message(STATUS "  ${FILE}")
endforeach()
message(STATUS "--------------------------------")

#Get list of all the full paths of all the compiled .spv files and compile them
set(SPIRV_OUTPUTS "")
foreach(SHADER_SRC ${SHADER_SOURCES})
    get_filename_component(SHADER_FILENAME ${SHADER_SRC} NAME)
    set(SHADER_SPV "${PROJECT_BINARY_DIR}/${SHADER_FILENAME}.spv")
    list(APPEND SPIRV_OUTPUTS ${SHADER_SPV})

    #Compile shader
    add_custom_command(
        OUTPUT ${SHADER_SPV}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER_SRC} -o ${SHADER_SPV}
        DEPENDS ${SHADER_SRC}
        COMMENT "Compiling ${SHADER_SRC} to ${SHADER_SPV}"
    )
endforeach()

#Create phony target that depends on all the compiled shaders and add it to the executable
add_custom_target(
    Shaders
    DEPENDS ${SPIRV_OUTPUTS}
)
add_dependencies(VulkanPBR Shaders)


#Copy all resource files to the output directory, maintaining subdirectory structure
file(COPY "${CMAKE_SOURCE_DIR}/Resource Files/" DESTINATION "${CMAKE_BINARY_DIR}/Resource Files/")