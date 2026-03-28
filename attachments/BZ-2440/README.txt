Server
 - rebinds object in NS
 - activates object
 - processes requests for 20 seconds
 - deactivates object
 - processes requests forever
 
Client
 - prompts user whether or not to use smart proxies
 - resolves object in NS
 - narrows to create (smart) proxy
 - periodically calls _non_existent() for 30 seconds
 - finally calls a user-defined CORBA method
 
Problems if smart proxies selected in client:
 - deactivation of object in server not detected by _non_existent() calls
 - a "orb->object_to_string(smart_proxy)" call provokes a MARSHAL exception