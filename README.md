# Zeno

You know what is Zeno.

At least now I know, tcp/udp on commodity NIC is VERY SLOW.  
It's like 240+ Kops per-core for small packets, leading to a ~15MB/s network bw.  

If I need to saturate 4GB/s, I need 64Mops 64B packet. As a comparasion, RDMA has ~7100 Kops per-core, leading to 0.42 GB/s.


## Compile

``` bash
mkdir build; cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
# or cmake -DCMAKE_BUILD_TYPE=Release ..
# for more options, see CMakeLists.txt
make -j8
```

## Run

``` bash
$ ./bin/main
zeno world!
```

## Install & Uninstall

``` bash
# to install libs and headers
sudo make install
# to uninstall
cat install_manifest.txt | sudo xargs rm -rf
```

## clang-format

`clang-format` is an opt-in if you would like to use.

``` bash
sudo apt install clang-format
```

## Doxygen

[doxygen](https://www.doxygen.nl/index.html) is supported if needed.

To generate a document and open it

``` bash
doxygen Doxyfile
open html/index.html
```
