# Description
This library contains classes for making communication in decoupled architecture.
- *Attribute*. Is used for notification listeners on its value changed. If you make local variable as attribute other classes may subscribe to its changes.
- *Event*. Is used for notification listeners. If you make local variable as event other classes may subscribe to your notifications.
- *CallHelper*. Contains helpers for ordered processing functions with callbacks.
- *SafeCaller*. Is used for prevent crashes on calling object's functions after the object have been destroyed.
- *Synched*. Is used for thread-safe access to wrapped object.
- *CallStrategy*. Is used for ordering/limiting blocks of calls (sequences).

# Usage examples
* [1_Simple_Attribute](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/1_Simple_Attribute/EntryPoint.cpp)
* [2_Simple_Event](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/2_Simple_Event/EntryPoint.cpp)
* [3_CallHelper](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/3_CallHelper/EntryPoint.cpp)
* [4_SafeCaller](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/4_SafeCaller/EntryPoint.cpp)
* [5_CallStrategy](https://github.com/darkessence87/psi-comm/blob/master/psi/examples/5_CallStrategy/EntryPoint.cpp)
