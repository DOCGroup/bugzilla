#include <tao/ORB.h>
#include <orbsvcs/CosNamingS.h>

int main(int argc, char **argv) {
	printf("Doing\n");
	for (int i = 0; i < 28; i++) {
		int argc = 2;
		char *argv[2];
		argv[0] = (char*) "-ORBInitRef";
		argv[1] = (char*) "NameService=corbaloc::localhost:3528/NameService";
		CORBA::ORB_var orbRef = CORBA::ORB_init(argc, argv, "server");
		CORBA::Object_var tmp_ref = orbRef->resolve_initial_references("NameService");
		CosNaming::NamingContextExt_var default_ctx =
				CosNaming::NamingContextExt::_narrow(tmp_ref);
		orbRef->shutdown(1);
		orbRef->destroy();
	}
	printf("Done\n");
	return 0;
}
