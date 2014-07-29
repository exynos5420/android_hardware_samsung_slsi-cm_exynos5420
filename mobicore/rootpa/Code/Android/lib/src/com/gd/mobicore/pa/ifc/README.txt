This file attempts to give generic guidelines on using RootPA. See from 
the .aidl files how to start up and use the services, from the .java 
files on details of particular components. Testing related information 
can be found from Locals/Test/Readme.txt.

When building RootPA, a rootpa-interface.jar is generated and copied to
Out/Bin/Release folder. This contains the com.gd.mobicore.pa.ifc 
package that is needed for implementing client that uses RootPA.

The actual RootPA is in RootPA-*.apk, * stands for release/debug 
signed/unsigned information. mdtest-RootPA*.apk contains RootPA that
uses MobiCore stub instead of real MobiCore. The stub accepts only 
some hardcoded values. Those can be found from Locals/Test/Readme.txt.


