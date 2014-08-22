Failover
========

# High Availablity
This is a project that try to implement the functionality of HA

# Requirement
* C++0x supported C++ compiler

# Build
A Makefile included, to build this project, just
```
make
```

# Concern on Asterisk
On Asterisk 11, after you compilation, installation, you should
```
sudo ldconfig
```
Because the asterisk can NOT find `libasteriskssl.so.1`
