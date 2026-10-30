cmake_minimum_required(VERSION 3.10.2)
project(CustomEngineNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# مسارات ملفات النواة
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/TokenBucketRateLimiter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/ChunkSlicer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/SplitReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/PriorityScheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/NativeBridge.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../core)

# بناء مكتبة مشتركة لنظام أندرويد (Shared Library .so)
add_library(custom_engine SHARED ${CORE_SOURCES})

# ربط مكتبات النظام الأساسية في أندرويد (خفيفة جداً ومناسبة للأجهزة الضعيفة)
find_library(log-lib log)
find_library(android-lib android)
find_library(gles3-lib GLESv3)

target_link_libraries(custom_engine
    ${log-lib}
    ${android-lib}
    ${gles3-lib}
)

# تحسينات الأداء لأجهزة أندرويد الاقتصادية (مثل معالجات Snapdragon 450 / Cortex-A53)
target_compile_options(custom_engine PRIVATE
    -O3
    -ffast-math
    -fvisibility=hidden
    -fexceptions
)
