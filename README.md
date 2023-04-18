# Intel_CET_Emulator

Intel's Control Enforcement Technology requires hardware capabilities that some processors are not equiped with. This emulator will enforce Intel's CET with Intel's Pin Tools

# Prerequisites
## Intel's ICC Compiler

In order to write a program that works with Intel's Control Enforcement Technology, it needs to be compiled with Intel's compiler ICC. The makefile for that program must also contain 3 flags:
* `-fcf-protection=full`
* `-mcet`
* `-msha`

To install Intel's ICC compiler, run the following commands:
 1. Set up the repository. To do this, download the key to the system keyring: 
 ```
 wget -O- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB \ | gpg --dearmor | sudo tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null
 ```

 2. Add the signed entry to APT sources and configure the APT client to use the Intel repository: 
 ```
 echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" | sudo tee /etc/apt/sources.list.d/oneAPI.list
 ```

 3. Update the packages list and repository index. 
 ```
 sudo apt update
 ```

 4. Install the compiler with APT:
 ```
 sudo apt install intel-basekit
 ```