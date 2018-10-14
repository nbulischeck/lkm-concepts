# lkm-concepts
Single-file LKM concepts

## Projects

* Base - Simple, but full example on a generic LKM implementation. No real functionality, but an excellent starting point.
* Keylogger - LKM Keylogger that uses a character device to retrieve the data logged.
* List-Dir - List userland directories from within the Kernel using Linux Dirent structures.
* Packet Stealing - LKM that forces a DoS on TCP packets by stealing them at the Netfilter prerouting hook.
* Get-Arch - LKM to retrieve the CPU's architecture based on whether x86_64 Long-Mode is present or not.

## Contributing

Create a new directory with a descriptive name of your example and fill it with a single `C` file and Makefile.

```
mkdir example
touch example.c
touch Makefile
```
