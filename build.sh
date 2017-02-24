set -ve

OUTDIRECTORY="linux"
CXXFLAGS="-Wall -O3 -std=c++1z"
LIBS="-lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lGLEW -pthread"
FADE2DLIBS=""

cd `dirname $0`

mkdir -p "${OUTDIRECTORY}"
cd "${OUTDIRECTORY}"

for f in ../Geo/*.cpp
do
    g++ ${CXXFLAGS} -c "$f"
done

g++ ${FADE2DLIBS} ${LIBS} *.o -o Geo
