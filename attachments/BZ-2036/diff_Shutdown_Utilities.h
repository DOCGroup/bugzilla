--- ACE-TAO/Shutdown_Utilities.h	2005-01-21 10:34:02.656354200 -0600
+++ Shutdown_Utilities.h	2004-08-25 18:05:42.000000000 -0500
@@ -24,7 +24,7 @@
   virtual void operator() (int which_signal) = 0;
 
 protected:
-  Shutdown_Functor();
+  Shutdown_Functor() { }
   virtual ~Shutdown_Functor() { }
 };
 
