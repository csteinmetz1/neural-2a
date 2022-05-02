# Neural-2A

## Build 

This will create a build for x86_64 only since libtorch provides binaries for this arch only.

```
cmake -G Xcode -B build -DCMAKE_PREFIX_PATH=/Users/cjstein/Code/neural-2a/libtorch
cd build
cmake --build . --config Release
```

### M1 Mac

On M1 Mac we have to build libtorch from source.

```
git clone -b master --recurse-submodule https://github.com/pytorch/pytorch.git
python3 -m venv env
source env/bin/activate
cd pytorch
pip install -r requirements.txt
cd ..
mkdir pytorch-build
mkdir libtorch
cd pytorch-build
cmake -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_BUILD_TYPE:STRING=Release -DPYTHON_EXECUTABLE:PATH=`whence -c python` -DCMAKE_INSTALL_PREFIX:PATH=../pytorch-install ../pytorch
cmake --build . --target install 
mv bin ../libtorch/
mv include ../libtorch/
mv lib ../libtorch/
mv share ../libtorch/
```
Notes
- Use `-DPYTHON_EXECUTABLE:PATH=`whence -c python` on zsh or `-DPYTHON_EXECUTABLE:PATH=`which python` on bash.


# JUCE CMake Repo Prototype
A prototype to model a way to create an entire repo using JUCE 6 and CMake.

This is inspired by a desire to keep the environment setting of my projects to minimum,
making sure the the environment is identical for every developer/machine.

The main concept is to set all the different variables (where JUCE is, custom modules, etc) 
in the top CMakeLists.txt, then add all your projects with very little setup time.

Another important concept is to share all 'related' projects under the same configuration,
which I prefer, since it encourages code-sharing and build system settings sharing.
In some of the examples I added minimal usages of juce-style modules to illustrate how that
can be done.

To build, all you have to do is load this project in your favorite IDE 
(CLion/Visual Studio/VSCode/etc) 
and click 'build' for one of the targets (templates, JUCE examples, Projucer, etc).

You can also generate a project for an IDE by using (Mac):
```
cmake -G Xcode -B build
```
Windows:
```
cmake -G "Visual Studio 16 2019" -B build
```
For package management, I'm using the amazing CPM.cmake:
#https://github.com/TheLartians/CPM.cmake
It automatically fetches JUCE from git, but you can also set the variable:
CPM_JUCE_SOURCE to point it to a local folder, by using:
``-DCPM_JUCE_SOURCE="Path_To_JUCE"``
when invoking CMake

JUCE can be found here:
#https://github.com/juce-framework/JUCE

License:
Anything from me in this repo is completely free to use for any purpose. 
However, please check the licenses for CPM and JUCE as described in their repo. 
