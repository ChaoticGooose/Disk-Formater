# Basic Disk Formatter
**DO NOT USE THIS ON ANY SYSTEM YOU CARE ABOUT. THIS IS A WORK IN PROGRESS TOOL AND MAY DESTROY DATA.**  
**I AM NOT RESPONSIBLE FOR ANY DATA LOSS OR DAMAGE TO YOUR SYSTEM.**  

A simple C disk formatter for Linux, and other Unix-like systems. This is a work in progress and is not yet fully functional.

## Usage
```
./format <filesystem> <target> [-q]
```
- Options:  
    - -q : quick format (Not yet implemented)  
- Positional arguments:  
    - filesystem : filesystem to format  
    - target : block device to format  


# Building
```
git clone https://github.com/ChaoticGooose/Disk-Formater.git
cd Disk-Formater
mkdir build
cd build
cmake ..
make
```
