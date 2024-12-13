# all: debug

debug: projectd
	cmake --build build --config debug -j14

release: projectr
	cmake --build build --config release -j14

projectd: CMakeLists.txt
	cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Debug

projectr: CMakeLists.txt
	cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

docs: Doxyfile
	echo Building docs to ./Docs/ ...
	doxygen
