export MAKESYSPATH=$HOME/os161/tools/share/mk/
cd asst2/src
./configure
bmake
cd kern/conf
./config ASST2
cd ../compile/ASST2
bmake depend
bmake
bmake install
cd ~/os161/root
sys161 kernel "p /testbin/asst2"
