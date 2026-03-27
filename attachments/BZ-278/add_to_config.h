	/*
	** Long Symbol Workaround -
	**	The following def's alias the templates that 
	**	TAO nests to unfathomable depths resulting in symbols
	**	in excess of 2000 characters
	*/
#define ALIAS_TEMPLATES
#ifdef ALIAS_TEMPLATES
	#define	ACE_Refcounted_Hash_Recyclable		T1
	#define TAO_IIOP_Client_Connection_Handler	T2
	#define ACE_Hash_Map_Manager				T3
	#define ACE_Hash_Map_Manager_Ex				T4
	#define ACE_Hash_Map_Iterator				T5
	#define ACE_Hash_Map_Iterator_Ex			T6
	#define ACE_Hash_Map_Reverse_Iterator_Ex	T7
	#define ACE_INET_Addr						T8
	#define ACE_Hash							T9
	#define	ACE_Pair							Ta
	#define	ACE_Null_Mutex						Tb
	#define ACE_Concurrency_Strategy			Tc
	#define ACE_Equal_To						Td
	#define TAO_Cached_Connector_Lock			Te
	#define TAO_UIOP_Client_Connection_Handler	Tf
#endif
#	ifndef __GNUC__
		#define ACE_HAS_TEMPLATE_TYPEDEFS			// No Help 
#	endif
	#define ACE_HAS_BROKEN_EXTENDED_TEMPLATES	// Some Help!
