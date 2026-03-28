--- ACE-TAO/Shutdown_Utilities.cpp	2005-01-21 09:24:12.135004800 -0600
+++ Shutdown_Utilities.cpp	2004-10-13 15:43:00.000000000 -0500
@@ -4,8 +4,6 @@
 
 #include "Shutdown_Utilities.h"
 
-Shutdown_Functor::Shutdown_Functor() { }
-
 Service_Shutdown::Service_Shutdown (Shutdown_Functor& sf)
   : functor_(sf)
 {
