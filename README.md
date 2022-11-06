# view-dependent-progressive-mesh-streaming
Những thứ cần tải:
 - gcc, g++
 - Freeglut
 - OpenMesh

### Installing OpenMesh
1) Extract it locally
2) Go into the OpenMesh-2.0 folder and make a new folder inside called 'build'
3) In your terminal, switch to the newly created 'build' folder: `cd /home/user/Downloads/OpenMesh-2.0/build/`
4) Configure the makefiles using cmake: `cmake ..` (include the two dots!)
5) Build the library: `make`
6) Move the all the compiled library files to /usr/local/lib/: `sudo mv /home/user/Downloads/OpenMesh-2.0/build/Build/lib/* /usr/local/lib/`
7) Move the folder with the header files we need to /usr/local/include/: `sudo mv /home/user/Downloads/OpenMesh-2.0/src/OpenMesh/ /usr/local/include/`
8) Run ldconfig so that your library loader can find it when running an application that needs the library: `sudo ldconfig -v`. To check, run `ldconfig -p | grep OpenMesh` and it should print a few library names containing OpenMeshCore and OpenMeshTools

```	
 libOpenMeshTools.so.2.0 (libc6,x86-64) => /usr/local/lib/OpenMesh/libOpenMeshTools.so.2.0
	libOpenMeshTools.so (libc6,x86-64) => /usr/local/lib/OpenMesh/libOpenMeshTools.so
	libOpenMeshCore.so.2.0 (libc6,x86-64) => /usr/local/lib/OpenMesh/libOpenMeshCore.so.2.0
	libOpenMeshCore.so (libc6,x86-64) => /usr/local/lib/OpenMesh/libOpenMeshCore.so
 ```
 Nếu mọi người chạy 2 dòng trên mà không ra thì chuyển file "OpenMesh.conf" vào folder /etc/ld.so.conf.d bằng dòng này: `sudo mv /home/user/Downloads/OpenMesh.conf -t /etc/ld.so.conf.d` sau 
 
### Installing Freeglut

(Mọi người xem cách tải ở đây ạ https://freeglut.sourceforge.net, bên dưới này là em cắt ở trong file /freeglut-3.4.0/README.cmake ra đấy :v)
- Make sure you have cmake installed. Examples:
  - Debian/Ubuntu: apt-get install cmake
  - Fedora: yum install cmake
  - FreeBSD: cd /usr/ports/devel/cmake && make install
  Or directly from their website:
  http://www.cmake.org/cmake/resources/software.html
- Make sure you have the basics for compiling code, such as C compiler
  (e.g., GCC) and the make package.
- Also make sure you have packages installed that provide the relevant
  header files for opengl (e.g., libgl1-mesa-dev on Debian/Ubuntu) and
  the chosen backend :
  - X11: x11 (e.g., libx11-dev, libxrandr-devel on Debian/Ubuntu) and
  XInput (libxi-dev / libXi-devel)
  - Wayland: wayland (e.g., libwayland-dev and libegl1-mesa-dev on
  Debian/Ubuntu) and xkbcommon (libxkbcommon-dev /libxkbcommon-devel)
- Run 'cmake .' (or 'cmake . -DFREEGLUT_WAYLAND=ON' for Wayland) in the
  freeglut directory to generate the makefile.
- Run 'make' to build, and 'sudo make install' to install freeglut.
- If you wish to change any build options run 'ccmake .'

### Build code cho bài báo
Mọi người chỉ cần chạy `make` trong file view-dependent-progressive-mesh-streaming-fixed là được ạ. 
