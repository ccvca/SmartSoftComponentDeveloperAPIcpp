# Generic SmartSoft Testing Framework

The SmartSoftTests is a generic and reusable testing framework that is designed to be (re-)used for
testing the different implementations of the abstract SmartSoft Component Developer API. Different
implementations might use completelly different middleware technologies internally, but these differences
should be hidden behind the API and all implementations should behave in the same way. In order to ensure
that the different middleware mappings behave the same, the SmartSoftTests have been developed such that
they can be mapped to your implementation easily by just subclassing and implementing the abstract methods
of the base class **TestingEnvironmentBase**. Besides of this class, all the tests can be used as is
without any changes.

In order to reuse these tests for your implementation, you have to follow these three steps:

1. Provide an implementation of the interface **TestingEnvironmentBase** where you 
   map the abstract pattern pointers to you pattern implementations
2. Implement the serialization of the **CommTestObjects** using your used middleware technology
3. Implement a simple main method that initializes the goolge tests

An example for each of these three steps can be found in the folder **FakeObjectsTests** which implements these
three parts for the FakeObjects implementation of the SmartSoft interface. The FakeObjectsTests provide a fully
fledged implementation that is executable and in fact is used to test the tests. :-)

The tests might be extended in future to cover even more corner cases, but for now they already cover the
majority of all standard cases.