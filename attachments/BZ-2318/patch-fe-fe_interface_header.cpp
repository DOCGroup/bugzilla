--- fe/fe_interface_header.cpp.orig	2005-11-25 14:27:34.000000000 +0600
+++ fe/fe_interface_header.cpp	2005-11-25 14:26:54.000000000 +0600
@@ -308,8 +308,8 @@
     }
 
   // Add i's parents to the flat list.
-  AST_Interface **parents = i->inherits ();
-  long num_parents = i->n_inherits ();
+  AST_Interface **parents = i->inherits_flat ();
+  long num_parents = i->n_inherits_flat ();
 
   for (long j = 0; j < num_parents; ++j)
     {
