# glow

## External dependencies

### stb ([`stb/`](stb/))
- URL: https://github.com/nothings/stb/
- License: [MIT, Unlicense](https://github.com/nothings/stb/blob/master/LICENSE)
- Upstream version: 1ee679c
  - `stb_image.h`: v2.27
- Local modifications:
  - Added a `std_impl.c` file:
    ```c
    // Implementation file for stb headers.

    #define STB_IMAGE_IMPLEMENTATION
    #define STBI_FAILURE_USERMSG
    #define STBI_NO_THREAD_LOCALS
    #include "stb_image.h"
    ```

### glfw ([`glfw/`](glfw/))
- URL: https://www.glfw.org/
- License: [Zlib](https://github.com/glfw/glfw/blob/master/LICENSE.md)
- Upstream version: 3.3.4
- Local modifications: None

### glad ([`glad/`](glad/))
- URL: https://glad.dav1d.de/#language=c&specification=gl&api=gl%3D3.3&api=gles1%3Dnone&api=gles2%3Dnone&api=glsc2%3Dnone&profile=core&extensions=GL_KHR_debug&loader=on
- License: [MIT, Apache 2.0](https://github.com/Dav1dde/glad/blob/master/LICENSE)
- Upstream version: 0.1.34
- Local modifications: None

### assimp ([`assimp/`](assimp/))
- URL: https://github.com/assimp/assimp
- License: [BSD-3-Clause](https://github.com/assimp/assimp/blob/master/LICENSE)
- Upstream version: 8748f85
- Local modifications: None
