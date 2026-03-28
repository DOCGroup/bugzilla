#if 0
gcc -g -I$ACE_ROOT -I$TAO_ROOT -I$TAO_ROOT/orbsvcs -L$ACE_ROOT/lib test.cpp -o test -lACE -lTAO -lTAO_PortableServer -lTAO_CosNaming -lTAO_IFR_Client
exit
#endif /* 0 */
/*
 * $Id: test.cpp $
 *
 * Author: Markus Stenberg <fingon@iki.fi>
 *
 *  Copyright (c) 2004 Markus Stenberg
 *       All rights reserved
 *
 * Created:       Wed Dec 29 13:55:55 2004 fingon
 * Last modified: Mon Jan 10 13:29:55 2005 mstenber
 * Edit time:     14 min
 *
 */

#include <tao/PortableServer/ORB_Manager.h>
#include <tao/IFR_Client/IFR_BasicC.h>
#include <orbsvcs/orbsvcs/CosNamingC.h>
#include <iostream>

using namespace std;

#define CQ_ASSERT(x)                                            \
do {                                                            \
    if (!(x))                                                   \
    {                                                           \
        cerr << "assertion failed at " << __LINE__ << endl;     \
        abort();                                                \
    }                                                           \
} while(0)

void test_ns(CORBA::ORB *orb)
{
    // Test that NS is correct
    CORBA::Object_var ns_var =
        orb->resolve_initial_references("NameService");

    CORBA::String_var str =
        orb->object_to_string(ns_var.in ()
                              ACE_ENV_ARG_PARAMETER);
    cout << "NS IOR is: " << str.in() << endl;

    CosNaming::NamingContext_var ns =
        CosNaming::NamingContext::_narrow(ns_var.in()
                                          ACE_ENV_ARG_PARAMETER);
    CQ_ASSERT(ns.in());
}

void test_ifr(CORBA::ORB *orb)
{
    // Test that IFR is correct
    CORBA::Object_var ifr_var =
        orb->resolve_initial_references("InterfaceRepository");
    CQ_ASSERT(ifr_var.in());

    CORBA::String_var str =
        orb->object_to_string(ifr_var.in()
                              ACE_ENV_ARG_PARAMETER);
    cout << "IFR IOR is: " << str.in() << endl;

    CORBA::Repository_var ifr =
        CORBA::Repository::_narrow(ifr_var.in()
                                   ACE_ENV_ARG_PARAMETER);
    CQ_ASSERT(ifr.in());
}

int main(int argc, char **argv)
{
    ACE_DECLARE_NEW_CORBA_ENV;

    ACE_TRY
    {
        TAO_ORB_Manager orb_manager;
        if (orb_manager.init_child_poa(argc, argv,
                                       "frontend-standalone") == - 1)
            ACE_ERROR_RETURN((LM_ERROR,
                              "Unable to initialize child POA %p\n",
                              "init_child_poa"),
                             -1);

        CORBA::ORB_var orb = orb_manager.orb();

        if (argc > 1)
        {
            test_ifr(orb);
            test_ns(orb);
        }
        else
        {
            test_ns(orb);
            test_ifr(orb);
        }
    }
    ACE_CATCH(CORBA::SystemException, sysex)
    {
        ACE_PRINT_EXCEPTION(sysex, "System Exception");
        return - 1;
    }
    ACE_CATCH(CORBA::UserException, userex)
    {
        ACE_PRINT_EXCEPTION(userex, "User Exception");
        return - 1;
    }
    ACE_ENDTRY;

    return 0;
}
