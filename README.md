# SmartSoft Component-Developer API for C++

## Overview

This repository contains the *Component-Developer C++ interface-definition (API)* for use of the *SmartSoft Framework*. Repository covers the API to the SmartSoft Communication Patterns and SmartSoft Component structures. Read more about the [SmartSoft World](http://www.servicerobotik-ulm.de/drupal/?q=node/7).

This API defines class hierarchies with pure virtual methods and specified template parameters. These pure virtual interface-definitions can be used to implement a Framework that conforms to the overall SmartSoft structures using any preferred middleware technology and/or operating-system abstraction. The API uses the *Standard-Template-Library (STD)* and conforms to the **C++11** standard. For available implementations of the SmartSoft Framework, please refer to [http://www.servicerobotik-ulm.de](http://www.servicerobotik-ulm.de).

## Communication Patterns and SmartSoft Component structures

Please find documentation on SmartSoft component structures in [Schlegel2004]. With respect to communication patterns, the API in this repository defines the inside view:

### Component Communication Patterns

* Send (Client/Server interaction)

  One-way communication.  
  Outside view: [Schlegel2004, pp. 85-88]

* Query (Client/Server interaction)

  Two-way request/response (request/response).  
  Outside view: [Schlegel2004, pp. 88-96]

* Push (Publisher/Subscriber interaction)

  1-n distribution.  
  Outside view: [Schlegel2004, pp. 96-99]

* Event (Publisher/Subscriber)

  1-n asynchronous conditioned notification  
  Outside view: [Schlegel2004, pp. 103-112]

### Coordination and Configuration Patterns

* Parameter (Master/Slave)

  Run-time configuration of components, see [Stampfer2016]  
  Outside view: [Lutz2014]

* State (Master/Slave)

  Lifecycle management and (de)activation  
  Outside view: [Schlegel2011]

* Dynamic Wiring (Master/Slave)

  Dynamic connection wiring at run-time  
  Outside view: [Schlegel2004, p. 11]

* Monitoring (Master/Slave)

  Run-time monitoring and introspection of components [Stampfer2016]  
  Outside view: [Lotz2011]

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


## References

Read more about the [SmartSoft World](http://www.servicerobotik-ulm.de/drupal/?q=node/7).

[Schlegel2004] C. Schlegel, Navigation and Execution for Mobile Robots in Dynamic Environments: An Integrated Approach, Ulm: Universität Ulm, 2004.

[Lutz2014] M. Lutz, D. Stampfer, A. Lotz und C. Schlegel, Service Robot Control Architectures for Flexible
and Robust Real-World Task Execution: Best Practices and Patterns, Stuttgart: Bonner Köllen
Verlag, 2014.

[Stampfer2016] D. Stampfer, A. Lotz, M. Lutz und C. Schlegel, „The SmartMDSD Toolchain: An Integrated MDSD Workflow and Integrated Development Environment (IDE) for Robotics Software,“ in
Journal of Software Engineering for Robotics (JOSER), 2016, pp. 3-19.

[Schlegel2011] C. Schlegel, A. Lotz und A. Steck, „SmartSoft: The State Management of a Component. Tech. rep.,“ 2011, University of Applied Sciences Ulm.

[Lotz2011] A. Lotz, A. Steck und C. Schlegel, „Runtime Monitoring of Robotics Software Components:
Increasing Robustness of Service Robotic Systems,“ in Proc. 15th Int. Conf. on Advanced
Robotics, Tallinn, Estland, 2011.
