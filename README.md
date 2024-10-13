# Android 编译 libjpeg-turbo

## 获取 libjpeg-turbo 源码

获取 [libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo) 源码这里使用 3.0.4

```bash
git clone -b 3.0.4 --depth 1 https://github.com/libjpeg-turbo/libjpeg-turbo.git libjpeg-turbo
```

## 编译 so包

编译好的so包会自动复制到 `./release` 目录下

```bash
# 编译 arm64-v8a
sh build.sh arm64-v8a
```

```bash
# 编译 armeabi-v7a
sh build.sh armeabi-v7a
```

```bash
# 编译 x86
sh build.sh x86
```

```bash
# 编译 x86_64
sh build.sh x86_64
```
