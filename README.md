# Description
This library contains classes for making communication in decoupled architecture.
- *[Attribute](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/Attribute.h)*. Is used for notification listeners on its value changed. If you make local variable as attribute other classes may subscribe to its changes.
- *[Event](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/Event.h)*. Is used for notification listeners. If you make local variable as event other classes may subscribe to your notifications.
- *[CallHelper](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/CallHelper.h)*. Contains helpers for ordered processing functions with callbacks.
- *[SafeCaller](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/SafeCaller.h)*. Is used for prevent crashes on calling object's functions after the object have been destroyed.
- *[Synched](https://github.com/darkessence87/psi-comm/blob/master/psi/include/psi/comm/Synched.h)*. Is used for thread-safe access to wrapped object.
- *[CallStrategy](https://github.com/darkessence87/psi-comm/tree/master/psi/include/psi/comm/call_strategy)*. Is used for ordering/limiting blocks of calls (sequences).

# Docs
[Diagrams](https://github.com/darkessence87/psi-comm/tree/master/psi/docs) created by [UMLet tool](https://www.umlet.com/)

# Usage examples
* [1_Simple_Attribute](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/1_Simple_Attribute/EntryPoint.cpp)
* [2_Simple_Event](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/2_Simple_Event/EntryPoint.cpp)
* [3_CallHelper](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/3_CallHelper/EntryPoint.cpp)
* [4_SafeCaller](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/4_SafeCaller/EntryPoint.cpp)
* [5_CallStrategy](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/5_CallStrategy/EntryPoint.cpp)
