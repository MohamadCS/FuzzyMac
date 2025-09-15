mkdir ./build/
cd ./build/
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build .
cd ..

killall FuzzyMac 2> /dev/null

