# SmartSoft FakeObjects implementation

This folder provides an exemplary implementation of the SmartSoft Component Developer API as Fake-Objects, 
which are used to execute and test the generic and reusable SmartSoft-Tests.
The implementation of Fake-Objects is fully fledged in the sense that all abstract interface methods are implemented
and all StatusCode values are handled correctly. Moreover, the different Pattern implementations simulate 
client-server connections and transfer data correctly between clients and servers over shared memory. In addition, 
even the typical asynchronous communication behavior is simmulated by using an extra internal thread for communication.

However, the FakeObjects should **not** be used as a productive middleware implementation because they have
several limitations (compared to a real middleware realization) that are irrelevant for the testing purposes.
Yet, the implementation of Fake-Objects can be used as inspiration for implementing and testing your own 
middleware mapping. If you do so, consider replacing the following fake implementation parts:
- Replace the FakeNaming service by a real one related to your middleware technology 
- Replace the FakeClientBase and FakeServerBase by more realistic implementations using your middleware technology
  that is actually able to communicate over the network and that uses real and more efficient and interoperable
  serialization mechanisms across different platforms, operating-systems and architectures
- The FakeComponent in your case might require the initialization of additional infrastructure
  specific to your used middleware technology
- Finally adjust all the Pattern implementations to use your implementations instead of the fake objects