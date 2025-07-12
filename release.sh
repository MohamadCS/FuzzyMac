cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd ..
killall FuzzyMac 2> /dev/null

