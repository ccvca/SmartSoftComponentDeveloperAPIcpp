# SmartSoft Component-Developer C++ API

## Overview

This repository contains the *Component-Developer C++ interface-definition (API)* for use of the *SmartSoft Framework*. This API defines class hierarchies with pure virtual methods and specified template parameters. These pure virtual interface-definitions can be used to implement a Framework that conforms to the overall SmartSoft structures using any preferred middleware technology and/or operating-system abstraction. The API uses the *Standard-Template-Library (STD)* and conforms to the **C++11** standard.

## Installation

The API is implemented as **headers only**. A CMake file is provided that creates an **Interface Library Target** (i.e. header only target). In order to use CMake, at the minimum the CMake version **3.1** is required. For using the library stand-alone, perform the following steps within the checked-out repository folder:

```
> mkdir build
> cd build
> cmake ..
> make
```

The API is shipped with a Doxygen documentation which can be automatically generated and viewed by additionally executing the following commands within the above created *build* folder:

```
> make doc
> firefox doc/html/index.html
```
