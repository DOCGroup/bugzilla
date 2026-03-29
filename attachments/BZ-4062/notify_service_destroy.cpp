#include <orbsvcs/Notify/Service.h>
#include <orbsvcs/Notify/EventChannelFactory.h>

#include <conio.h>
#include <ctype.h>

/// In constructor init and run ORB, in destructor stop and destroy ORB
class OrbManager
{
public:
     /// Init and run ORB
     OrbManager()
     {
          const char* orbArgv[] = {
               "-ORBListenEndpoints", "iiop://127.0.0.1:56743"
          };
          int orbArgc = sizeof(orbArgv) / sizeof(orbArgv[0]);     

          orb_ = CORBA::ORB_init(orbArgc, const_cast<char**>(orbArgv) );
          
          CORBA::Object_var objRootPoa = orb_->resolve_initial_references("RootPOA");
          PortableServer::POA_var rootPoa = PortableServer::POA::_narrow(objRootPoa);
          PortableServer::POAManager_var poaMgr = rootPoa->the_POAManager();
          poaMgr->activate();
          
          ACE_Thread::spawn( OrbManager::RunOrb, this, THR_NEW_LWP | THR_JOINABLE, &orbThreadId_ );
     }

     /// Stop and destroy ORB
     ~OrbManager()
     {
          CORBA::Object_var objRootPoa = orb_->resolve_initial_references("RootPOA");
          PortableServer::POA_var rootPoa = PortableServer::POA::_narrow(objRootPoa);
          PortableServer::POAManager_var poaMgr = rootPoa->the_POAManager();
          poaMgr->deactivate( true, true );

          orb_->shutdown( true );

          ACE_Thread::join( orbThreadId_, 0, 0 );

          orb_->destroy();
     }
     
     CORBA::ORB_ptr GetOrb()
     {
          return CORBA::ORB::_duplicate( orb_.in() );
     }
     
private:
     static ACE_THR_FUNC_RETURN RunOrb(void* orbManager )
     {
          OrbManager* this_ = static_cast< OrbManager* >( orbManager );
          this_->orb_->run();
          return 0;
     }
     
     ACE_thread_t orbThreadId_;
     CORBA::ORB_var orb_;
};

/// In constructor create and run Notification Service, in destructor stop and destroy Notification Service
class NotifyServiceManager
{
public:
     /// Create and run Notification Service
     NotifyServiceManager( CORBA::ORB_ptr orb ): orb_( orb )
     {
          notify_service_ = TAO_Notify_Service::load_default ();
          notify_service_->add_reference();

          notify_service_->init( 0, 0 );
          notify_service_->init_service( orb_.in() );

          CORBA::Object_var objRootPoa = orb_->resolve_initial_references("RootPOA");
          PortableServer::POA_var rootPoa = PortableServer::POA::_narrow(objRootPoa);
          eventChannelFactory_ = notify_service_->create( rootPoa.in () );
     }
     
     /// Stop and destroy Notification Service
     ~NotifyServiceManager()
     {
          this->notify_service_->finalize_service( eventChannelFactory_.in () );
          
          // It is TAO-specific interface. May use destroy() as workaround of error during POA Manager deactivation.
          //NotifyExt::EventChannelFactory_var eventChannelFactoryExt = NotifyExt::EventChannelFactory::_narrow( eventChannelFactory_ );
          //eventChannelFactoryExt->destroy();
          
          this->notify_service_->fini();
     }

private:
     
     CORBA::ORB_var orb_;
     TAO_Notify_Service* notify_service_;     
     CosNotifyChannelAdmin::EventChannelFactory_var eventChannelFactory_;
};

int main( int argc, char* argv[] )
{
     OrbManager orbManager;
     CORBA::ORB_var orb = orbManager.GetOrb();
     NotifyServiceManager notifyServiceManager( orb.in() );
     
     Sleep( 1000 );
          
     return 0;
}