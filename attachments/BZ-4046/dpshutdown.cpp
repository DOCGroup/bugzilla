#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/RTPS/RtpsDiscovery.h>

int ACE_TMAIN (int argc, ACE_TCHAR *argv[]){
  ACE_DEBUG ((LM_DEBUG, "(%P|%t|%T) Starting test\n"));
  try
    {
      // This sleep will make core later
      //      ACE_OS::sleep(2);
      DDS::DomainParticipantFactory_var dpf =
        TheParticipantFactoryWithArgs(argc, argv);

      OpenDDS::RTPS::RtpsDiscovery_rch disc =
        new OpenDDS::RTPS::RtpsDiscovery(OpenDDS::DCPS::Discovery::DEFAULT_RTPS);

      TheServiceParticipant->add_discovery(OpenDDS::DCPS::static_rchandle_cast<OpenDDS::DCPS::Discovery>(disc));
      TheServiceParticipant->set_default_discovery (OpenDDS::DCPS::Discovery::DEFAULT_RTPS);

      DDS::DomainParticipant_var participant =
        dpf->create_participant(11,
                                PARTICIPANT_QOS_DEFAULT,
                                DDS::DomainParticipantListener::_nil(),
                                ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
      if (CORBA::is_nil (participant.in ())) {
        ACE_ERROR ((LM_ERROR, ACE_TEXT("create_participant failed.")));
        return 1;
      }
      else
      {
        //        ACE_OS::sleep(2);
      }


      DDS::DomainParticipant_var participant2 =
        dpf->create_participant(11,
                                PARTICIPANT_QOS_DEFAULT,
                                DDS::DomainParticipantListener::_nil(),
                                ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
      if (CORBA::is_nil (participant2.in ())) {
        ACE_ERROR ((LM_ERROR, ACE_TEXT("create_participant failed.")));
        return 1;
      }


      DDS::ReturnCode_t retcode6 = dpf->delete_participant(participant.in ());
      if (retcode6 != DDS::RETCODE_OK) {
        ACE_ERROR_RETURN((LM_ERROR,
                          ACE_TEXT("%N:%l: main()")
                          ACE_TEXT(" ERROR: should be able to delete participant\n")),
                        -1);
      }

      ACE_DEBUG ((LM_DEBUG, "Shuting down tsp with one participant still registered\n"));
      TheServiceParticipant->shutdown ();
  }
  catch (CORBA::Exception& e)
  {
    e._tao_print_exception("ERROR: Exception thrown:");
    exit(1);
  }
  ACE_DEBUG((LM_INFO, "(%P|%t|%T) Test finish\n"));
  return 0;
}
