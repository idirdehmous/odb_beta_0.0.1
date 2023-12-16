gcc -pthread -Wno-unused-result -Wsign-compare -DDYNAMIC_ANNOTATIONS_ENABLED=1 \
-DNDEBUG -O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions  \
-fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches -m64  \
-mtune=generic -D_GNU_SOURCE -fPIC -fwrapv -fPIC -I/usr/include/python3.8 -c src/lib/add.c -o ./mymath.o


gcc -pthread -shared -Wl,-z,relro -g ./mymath.o -L/usr/lib64 -lm -lpython3.8 -o ./_mymath.cpython-36m-x86_64-linux-gnu.so
