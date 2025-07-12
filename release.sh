rm -r release
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd ..
killall FuzzyMac 2> /dev/null
cp -r ./release/build/app/FuzzyMac.app /Applications
open -a "FuzzyMac"

