# Limits the CPU consumption of the top process, executes uiautomator dump and converts the xml to csv

## How to install 

### 1. Root your Android device / emulator 

### 2. Install Termux and [break out of the Termux environment](https://github.com/hansalemaos/termuxfree)

### 3. Install cpulimit 

```sh 
pkginstall cpulimit #breakout version
pkg install cpulimit
```

### 4. Install tur-repo 

```sh 
pkginstall tur-repo #breakout version
pkg install tur-repo
```
### 5. Install gcc 14
```sh 
pkginstall gcc-14 #breakout version
pkg install gcc-14

```

### 6. Compile the C++ code 

```sh 
g++-14 -std=c++20 -O3 ./uiautomatorcpulimit.cpp -o ./uiautomatorcpulimitexe
```

### 7. Execute

```sh 
# remove the old dump (if any)
rm -f /sdcard/window_dump.xml

# The first argument the max cpu consumption 
# If you are getting something like:
# ERROR: null root node returned by UiTestAutomationBridge.
# you are limiting too much
# The secong argument the the priority of uiautomator 0 (normal) until 19 (max)

./uiautomatorcpulimitexe 5 18
```



