../build.linux/nachos -f
../build.linux/nachos -mkdir /hello/
../build.linux/nachos -mkdir /hello/world/
../build.linux/nachos -mkdir /hello/world/i/
../build.linux/nachos -mkdir /hello/world/i/am/
../build.linux/nachos -mkdir /hello/world/i/am/brayce/

../build.linux/nachos -lr /
echo "----------------------"


../build.linux/nachos -cp num_100.txt /hello/world/i/am/brayce/10
../build.linux/nachos -cp num_100.txt /hello/world/i/am/10
../build.linux/nachos -cp num_100.txt /hello/world/i/10
../build.linux/nachos -cp num_100.txt /hello/world/10

../build.linux/nachos -lr /
echo "----------------------"

../build.linux/nachos -rr /hello/world/i/
../build.linux/nachos -lr /
echo "----------------------"
