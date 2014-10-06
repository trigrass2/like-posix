minstdlibs
==========

tiny implementations of c standard library functions


Testing
-------

Setting up gtest

```
# acquire, build and install gtest
wget http://googletest.googlecode.com/files/gtest-1.7.0.zip
unzip gtest-1.7.0.zip
cd gtest-1.7.0
./configure
make
sudo cp -a include/gtest /usr/include
sudo cp -a lib/.libs/* /usr/lib/
sudo ldconfig -v | grep gtest
# check that the gtest so's are in place
libgtest.so.0 -> libgtest.so.0.0.0
libgtest_main.so.0 -> libgtest_main.so.0.0.0
# optionally remove gtest sources
cd ..
rm -rf gtest-1.7.0

```

Building and runnning the tests

```
cd minstdlibs/test
make clean all run
```
