#!/bin/bash

# 检查是否传递了 ABI 参数
if [ -z "$1" ]; then
  echo "请提供 ABI 参数，例如: arm64-v8a, armeabi-v7a, x86, x86_64"
  exit 1
fi

# 获取传递的 ABI 参数
ABI=$1

# 确定输出目录
OUTPUT_DIR="$(pwd)/libjpeg/src/main/jniLibs/$ABI"
mkdir -p "$OUTPUT_DIR"

# libjpeg-turbo 源码路径
LIBJPEG_TURBO_SRC="$(pwd)/libjpeg-turbo"

# 设置 Android NDK 的路径
# 你需要将 ANDROID_NDK 设置为你的 NDK 路径
ANDROID_NDK=/Users/tangjw/Library/Android/sdk/ndk/25.1.8937393
ANDROID_VERSION=21  # 最低支持 api-21

CMAKE_OPTIONS="cmake -G\"Unix Makefiles\" -DANDROID_ABI=$ABI -DANDROID_PLATFORM=android-$ANDROID_VERSION -DANDROID_TOOLCHAIN=clang -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake"

# 如果 ABI 是 arm64-v8a，添加 -DCMAKE_ASM_FLAGS
if [ "$ABI" == "arm64-v8a" ]; then
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DANDROID_ARM_MODE=arm -DCMAKE_ASM_FLAGS="--target=aarch64-linux-android$ANDROID_VERSION""
elif [ "$ABI" == "armeabi-v7a" ]; then
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DANDROID_ARM_MODE=arm -DCMAKE_ASM_FLAGS="--target=arm-linux-androideabi$ANDROID_VERSION""
fi

# 清理之前的构建
rm -rf build
mkdir build
cd build || exit

# 执行 cmake 命令，配置编译选项
eval "$CMAKE_OPTIONS $LIBJPEG_TURBO_SRC"

# 编译
# make
make -j$(nproc)

# 将生成的 .so 文件拷贝到指定目录
if [ -f ./libturbojpeg.so ]; then
  cp ./libturbojpeg.so "$OUTPUT_DIR"/
  echo "生成的 .so 文件已复制到 $OUTPUT_DIR 文件夹下。"
else
  echo "未找到生成的 .so 文件，检查编译是否成功。"
fi
