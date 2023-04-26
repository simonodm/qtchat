# QtChat

QtChat is a peer-to-peer instant messaging application with support for configurable user information, message editing and RSA/AES message encryption.

## Compilation

Compiling QtChat requires CMake, [Qt Framework](https://www.qt.io/) version 6.2.4+, and a C++ compiler.

The application can then be compiled as follows:

```bash
$> mkdir qtchat-build
$> cd qtchat-build
$> cmake
    -DBUILD_SHARED_LIBS=OFF # if building with static linking
    -DCMAKE_PREFIX_PATH=C:\\Qt624Static # path to your Qt installation if building with static linking
    -G Ninja # or a different build tool
    # additional parameters such as CMAKE_MAKE_PROGRAM, CMAKE_C_COMPILER, CMAKE_CXX_COMPILER, CMAKE_TOOLCHAIN_FILE can be set here
```

After configuration is done, run the following command in the build directory:

```bash
$> cmake --build .
```

The result of the above should be a single binary containing the full application.