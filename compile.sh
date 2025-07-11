cd ./build/
cmake ..
cmake --build .
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .
cd ..

killall FuzzyMac 2> /dev/null

