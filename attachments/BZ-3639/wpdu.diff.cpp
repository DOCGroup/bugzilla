--- /vobs/vendor/ACE_wrappers/ASNMP/asnmp/wpdu.cpp@@/main/ace567vendor/1        2008-11-27 16:00:00.000000000 +0200
+++ /vobs/vendor/ACE_wrappers/ASNMP/asnmp/wpdu.cpp      2009-04-06 08:33:10.000000000 +0300
@@ -457,6 +457,14 @@
     }
     break;
  
+    // Gauge32
+    case sNMP_SYNTAX_GAUGE32:
+    {
+       Gauge32 gauge32( (unsigned long) *(vp->val.integer));
+       tempvb.set_value( gauge32);
+    }
+    break;
+
     // 32 bit counter
     case sNMP_SYNTAX_CNTR32:
     {

