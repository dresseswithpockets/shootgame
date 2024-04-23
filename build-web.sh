emcc -o src/pch.h.pch src/pch.h \
    -O2 \
    -Wall \
    -Iinclude \
    -s USE_GLFW=3 \
    --shell-file src/shell.html \
    --preload-file assets \
    -s TOTAL_STACK=64MB \
    -s INITIAL_MEMORY=128MB \
    -s ASSERTIONS \
    -DPLATFORM_WEB \
    -Xclang -emit-pch

emcc -o ./bin/index.html src/main.c src/game.c src/entity.c lib/webassembly/libraylib.a \
    -O2 \
    -Wall \
    -Iinclude \
    -s USE_GLFW=3 \
    --shell-file src/shell.html \
    --preload-file assets \
    -s TOTAL_STACK=64MB \
    -s INITIAL_MEMORY=128MB \
    -s ASSERTIONS \
    -DPLATFORM_WEB \
    -include-pch src/pch.h.pch