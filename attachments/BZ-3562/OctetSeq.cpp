// $Id: OctetSeq.cpp 82733 2008-09-16 11:54:19Z johnnyw $

// ============================================================================
//
// = LIBRARY
//    TAO/tests/OctetSeq
//
// = FILENAME
//    octetseq.cpp
//
// = DESCRIPTION
//   TAO optimizes octet sequences, this test verifies that the
//   optimizations do not break any code and that they effectively
//   improve performance.
//
// = AUTHORS
//    Carlos O'Ryan
//
// ============================================================================

#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"

#include "tao/ORB.h"
#include "tao/CDR.h"
#include "testC.h"
#include "ace/OS_NS_string.h"
#include "tao/ORB_Core.h"

ACE_RCSID(OctetSeq, OctetSeq, "$Id: OctetSeq.cpp 82733 2008-09-16 11:54:19Z johnnyw $")

TAO_ORB_Core * orb_core;

static int
test_write_octet (TAO_OutputCDR &cdr,
                  char* /* buf*/,
                  size_t bufsize)
{
#if (TAO_NO_COPY_OCTET_SEQUENCES == 1)

  ACE_Message_Block mb (bufsize);
  Test::OctetSeq os (bufsize, &mb);
#else
  Test::OctetSeq os (bufsize);
  os.length (bufsize);
#endif /* TAO_NO_COPY_OCTET_SEQUENCES == 1 */


  if ((cdr << os) == 0)
    return -1;
  return 0;
}

static int
test_read_octet (TAO_InputCDR &cdr,
                 char* /*buf*/,
                 size_t /*bufsize*/)
{
  Test::OctetSeq os;

  if ((cdr >> os) == 0)
  {
    printf("demarshal_sequence failed\n");
    return -1;
  }
  return 0;
}

typedef
int (*Writer)(TAO_OutputCDR& cdr,
              char* buf, size_t bufsize);
typedef
int (*Reader)(TAO_InputCDR& cdr,
              char* buf, size_t bufsize);

int
run (char* buf, size_t bufsize,
     size_t n, size_t lo, size_t s,
     int quiet,
     const ACE_TCHAR* name,
     Writer writer, Reader reader)
{
  size_t count = 0;
  double sum_read = 0;
  double sum_write = 0;

  ACE_Data_Block * db =
     new ACE_Data_Block(0, ACE_Message_Block::MB_DATA,
        0, 0, 0, 0, 0);

   ACE_Message_Block * mb =
      new ACE_Message_Block(db);
 
   {
      db->base(buf, bufsize+sizeof(CORBA::ULong));
      mb->base(buf, bufsize+sizeof(CORBA::ULong));
      mb->wr_ptr ((char *)mb->base() + bufsize+sizeof(CORBA::ULong));
      mb->rd_ptr((char *)mb->base());
      mb->clr_self_flags(ACE_Message_Block::DONT_DELETE);
      mb->clr_flags(ACE_Message_Block::DONT_DELETE);
      db->clr_flags(ACE_Message_Block::DONT_DELETE);
      TAO_OutputCDR output(mb);

      if (writer (output, buf, bufsize) != 0)
      {
         printf("writer error\n");
         return -1;
      }

      printf("TAO_OutputCDR buffer at %p\n", output.buffer());

      mb->wr_ptr ((char *)mb->base() + bufsize+sizeof(CORBA::ULong));
      mb->rd_ptr((char *)mb->base());
      TAO_InputCDR input (output.buffer(), bufsize+sizeof(CORBA::ULong), ACE_CDR_BYTE_ORDER, TAO_DEF_GIOP_MAJOR, TAO_DEF_GIOP_MINOR, orb_core);
      const_cast<ACE_Message_Block *>(input.start())->clr_self_flags(ACE_Message_Block::DONT_DELETE);
      const_cast<ACE_Message_Block *>(input.start())->clr_flags(ACE_Message_Block::DONT_DELETE);
      printf("TAO_InputCDR rd_ptr at %p\n", input.rd_ptr());

      Test::OctetSeq os;

      if ((input >> os) == 0)
      {
         printf("demarshal_sequence failed\n");
         return -1;
      }

      printf("demarshalled octet sequence buffer at %p\n", os.get_buffer());
      mb->set_self_flags(ACE_Message_Block::DONT_DELETE);
      mb->set_flags(ACE_Message_Block::DONT_DELETE);
      db->set_flags(ACE_Message_Block::DONT_DELETE);
      const_cast<ACE_Message_Block *>(input.start())->set_self_flags(ACE_Message_Block::DONT_DELETE);
      const_cast<ACE_Message_Block *>(input.start())->set_flags(ACE_Message_Block::DONT_DELETE);
   }

  for (size_t x = lo; x <= bufsize; x += s)
    {
      ACE_High_Res_Timer writing;
      ACE_High_Res_Timer reading;

      for (size_t i = 0; i < n; ++i)
        {
          writing.start_incr ();
          db->base(buf, bufsize+sizeof(CORBA::ULong));
          mb->base(buf, bufsize+sizeof(CORBA::ULong));
          mb->wr_ptr ((char *)mb->base() + bufsize+sizeof(CORBA::ULong));
          mb->rd_ptr((char *)mb->base());
          mb->clr_self_flags(ACE_Message_Block::DONT_DELETE);
          mb->clr_flags(ACE_Message_Block::DONT_DELETE);
          db->clr_flags(ACE_Message_Block::DONT_DELETE);
          TAO_OutputCDR output(mb);

          if (writer (output, buf, x) != 0)
          {
            printf("writer error\n");
            return -1;
          }
          writing.stop_incr ();
          mb->wr_ptr ((char *)mb->base() + bufsize+sizeof(CORBA::ULong));
          mb->rd_ptr((char *)mb->base());
          TAO_InputCDR input (output.buffer(), bufsize+sizeof(CORBA::ULong), ACE_CDR_BYTE_ORDER, TAO_DEF_GIOP_MAJOR, TAO_DEF_GIOP_MINOR, orb_core);
          const_cast<ACE_Message_Block *>(input.start())->clr_self_flags(ACE_Message_Block::DONT_DELETE);
          const_cast<ACE_Message_Block *>(input.start())->clr_flags(ACE_Message_Block::DONT_DELETE);
          reading.start_incr ();
          if (reader (input, buf, x) != 0)
          {
            printf("reader error\n");
            return -1;
          }
          reading.stop_incr ();
          mb->set_self_flags(ACE_Message_Block::DONT_DELETE);
          mb->set_flags(ACE_Message_Block::DONT_DELETE);
          db->set_flags(ACE_Message_Block::DONT_DELETE);
          const_cast<ACE_Message_Block *>(input.start())->set_self_flags(ACE_Message_Block::DONT_DELETE);
          const_cast<ACE_Message_Block *>(input.start())->set_flags(ACE_Message_Block::DONT_DELETE);
        }

      double m = n;

      ACE_Time_Value wtv;
      writing.elapsed_time_incr (wtv);
      ACE_hrtime_t wusecs = wtv.sec ();
      wusecs *= static_cast<ACE_UINT32> (ACE_ONE_SECOND_IN_USECS);
      wusecs += wtv.usec ();

      ACE_Time_Value rtv;
      reading.elapsed_time_incr (rtv);
      ACE_hrtime_t rusecs = rtv.sec ();
      rusecs *= static_cast<ACE_UINT32> (ACE_ONE_SECOND_IN_USECS);
      rusecs += rtv.usec ();

      double write_average = ACE_HRTIME_CONVERSION(wusecs) / m;
      double read_average = ACE_HRTIME_CONVERSION(rusecs) / m;

      count++;
      sum_read += read_average;
      sum_write += write_average;
      if (quiet == 0)
        {
          ACE_DEBUG ((LM_DEBUG,
                      "%s: %d %.3f %.3f\n",
                      name, x, write_average, read_average));
        }
    }

  if (count != 0)
    {
      ACE_DEBUG ((LM_DEBUG,
                  "%s total: %.3f %.3f\n",
                  name, sum_write / count, sum_read / count));
    }
  delete mb;
  delete db;
  return 0;
}

int
ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
  try
    {
      CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
      orb_core = orb->orb_core();

      int n = 64;
      int lo = 128;
      int hi = 65536;
      int s = 32;

      int quiet = 0;

      ACE_Get_Opt get_opt (argc, argv, ACE_TEXT("qn:l:h:s:"));
      int opt;

      while ((opt = get_opt ()) != EOF)
        {
          switch (opt)
            {
            case 'n':
              n = ACE_OS::atoi (get_opt.opt_arg ());
              break;
            case 'l':
              lo = ACE_OS::atoi (get_opt.opt_arg ());
              break;
            case 'h':
              hi = ACE_OS::atoi (get_opt.opt_arg ());
              break;
            case 's':
              s = ACE_OS::atoi (get_opt.opt_arg ());
              break;
            case 'q':
              quiet = 1;
              break;

            case '?':
            default:
              ACE_DEBUG ((LM_DEBUG,
                          "Usage: %s "
                          "-l low "
                          "-h high "
                          "-s step "
                          "-n n "
                          "\n"
                          "Writes and then reads octet sequences to a CDR stream "
                          "starting from <low> up to <high> incrementing "
                          "by <step>, at each step run <n> iterations to "
                          "average."
                          "\n",
                          argv[0]));
              return -1;
            }
        }

      ACE_DEBUG ((LM_DEBUG, "Running:\n"
                  "  low: %d\n"
                  "  hi : %d\n"
                  "  s  : %d\n"
                  "  n  : %d\n",
                  lo, hi, s, n));

      // Create a "big" buffer and fill it up.
      char* buf = new char[hi + sizeof(CORBA::ULong)];
      CORBA::Long l = 0xdeadbeef;
      for (int i = 0; i < hi / (int) sizeof (l); ++i)
        {
          ACE_OS::memcpy (buf + sizeof (l) * i, &l, sizeof (l));
        }
      printf("CDR buffers should start at %p\n", buf);
      printf("Octet Sequence buffer should start at %p\n", &buf[sizeof(CORBA::ULong)]);
      if (run (buf, hi,
               n, lo, s, quiet,
               ACE_TEXT ("OCTET"), test_write_octet, test_read_octet) != 0)
        return 1;
      delete[] buf;
    }
  catch (const CORBA::Exception& ex)
    {
      ex._tao_print_exception ("OctetSeq");
    }

  return 0;
}
