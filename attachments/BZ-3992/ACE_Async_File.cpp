// ACE_Async_File.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ace/Asynch_IO.h"
#include "ace/SString.h"
#include "ace/Proactor.h"
#include "ace/OS.h"
#include "ace/FILE_Connector.h"
#include "ace/FILE_IO.h"
#include "ace/FILE_Addr.h"
#include "ace/Message_Block.h"

class CHandler : public ACE_Handler
{
protected:
	ACE_FILE_IO				m_DestFileIO;
	ACE_Asynch_Write_File   m_AsynchWF;
	int						m_iCount;
	int						m_iMaxCount;
	int						m_iBlockSize;
	size_t					m_ulOffset;
	int						m_iPendingWrites;
	
	int writeNextBlock(void)
	{
		ACE_Message_Block *pMB = 0;			
		ACE_NEW_RETURN(pMB, ACE_Message_Block(m_iBlockSize), -1);
		ACE_OS::memset(pMB->wr_ptr(), 0x41, pMB->space());
		pMB->wr_ptr(pMB->space());

		ACE_DEBUG((LM_INFO, 
			ACE_TEXT("Scheduling offset %u, pending %i\n"),
			m_ulOffset,
			m_iPendingWrites));
		m_iPendingWrites++;
		if (m_AsynchWF.write(*pMB, pMB->size(), m_ulOffset,0))
		{
			ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Error: writing block at offset %u\n    %m\n"),
				m_ulOffset),
				-1);
		}
		m_ulOffset += m_iBlockSize;
	
		return 0;
	}

public:
	virtual int init(const ACE_TString &strFile)
	{
		proactor(ACE_Proactor::instance());
		m_iCount = 0;
		m_iMaxCount = 10;
		m_iBlockSize =  ACE_OS::getpagesize ();


		ACE_FILE_Connector con;
		ACE_FILE_Addr dest_addr(strFile.c_str());

		    // open destination file for writing
		if (con.connect (m_DestFileIO, dest_addr, 0, ACE_Addr::sap_any, 0,
#ifdef ACE_WIN32    
			(O_CREAT | O_TRUNC | O_WRONLY | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED)) == -1)
#else        
			(O_CREAT | O_TRUNC | O_WRONLY)) == -1)
#endif // ACE_WIN32        
		{
			ACE_ERROR_RETURN ((LM_ERROR,
				"%p\n", "ACE_FILE_Connector::connect (write)"), -1);      

		}


		// Open ACE_Asynch_Write_File
		if (m_AsynchWF.open (*this, m_DestFileIO.get_handle (), 0, proactor ()) == -1)
		{
			ACE_ERROR_RETURN ((LM_ERROR,
				"%p\n", "ACE_Asynch_Write_File::open"), -1);
		}

		//schedule some writes
		m_ulOffset = 0;
		m_iPendingWrites = 0;
		for (int iCompletion = 0; iCompletion < 4; ++iCompletion)
		{
			if (writeNextBlock())
			{
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("Error: writing block %i.\n"),
					iCompletion),
					-1);
					
			}
		}

		return 0;

	}

	

	virtual void handle_write_file (const ACE_Asynch_Write_File::Result &result)
	{
		m_iCount++;
		m_iPendingWrites--;

		if (result.error())
		{
			ACE_ERROR((LM_ERROR,
				ACE_TEXT("handle_write_file::error %u\n"),
				result.error()));
		}

		result.message_block().release();

		if (m_iCount < m_iMaxCount)
		{
			//schedule a new one
			writeNextBlock();
		}

		if (m_iPendingWrites == 0)
		{	
			proactor()->proactor_end_event_loop();
			ACE_DEBUG((LM_INFO, ACE_TEXT("Finished writing %u bytes\n"),
				m_ulOffset));
		}

	}
	
};


int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	CHandler h;

	if (h.init(ACE_TEXT("test.dat")))
	{
		ACE_ERROR_RETURN((LM_ERROR,
			ACE_TEXT("Error: initializing handler\n")),
			-1);
	}

	ACE_Proactor::instance()->proactor_run_event_loop();
	
	return 0;
}

