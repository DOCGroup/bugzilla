// $Id: ECG_Mcast_EH.cpp,v 1.13 2002/06/26 14:20:34 crodrigu Exp $

#include "orbsvcs/Event/ECG_Mcast_EH.h"

#if defined (ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION)

template class ACE_Hash_Map_Manager<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast*, ACE_Null_Mutex>;
template class ACE_Hash_Map_Manager_Ex<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast *, ACE_Hash<ACE_INET_Addr>, ACE_Equal_To<ACE_INET_Addr>, ACE_Null_Mutex>;
template class ACE_Hash_Map_Entry<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast *>;
template class ACE_Hash<ACE_INET_Addr>;
template class ACE_Node<ACE_INET_Addr>;
template class ACE_Equal_To<ACE_INET_Addr>;
template class ACE_Hash_Map_Iterator_Base_Ex<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast*, ACE_Hash<ACE_INET_Addr>, ACE_Equal_To<ACE_INET_Addr>, ACE_Null_Mutex>;
template class ACE_Hash_Map_Iterator<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast*, ACE_Null_Mutex>;
template class ACE_Hash_Map_Iterator_Ex<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast*, ACE_Hash<ACE_INET_Addr>, ACE_Equal_To<ACE_INET_Addr>, ACE_Null_Mutex>;
template class ACE_Hash_Map_Reverse_Iterator<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast*, ACE_Null_Mutex>;
template class ACE_Hash_Map_Reverse_Iterator_Ex<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast*, ACE_Hash<ACE_INET_Addr>, ACE_Equal_To<ACE_INET_Addr>, ACE_Null_Mutex>;

template class TAO_ECG_Mcast_EH_T<ACE_SYNCH_MUTEX>;

template class ACE_Hash_Map_Manager<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Null_Mutex>;
template class ACE_Hash_Map_Manager_Ex<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Hash<ACE_HANDLE>, ACE_Equal_To<ACE_HANDLE>, ACE_Null_Mutex>;
template class ACE_Hash_Map_Entry<ACE_HANDLE, TAO_Mcast_Socket* >;
template class ACE_Hash<ACE_HANDLE>;
template class ACE_Node<ACE_HANDLE>;
template class ACE_Equal_To<ACE_HANDLE>;
template class ACE_Hash_Map_Iterator_Base_Ex<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Hash<ACE_HANDLE>, ACE_Equal_To<ACE_HANDLE>, ACE_Null_Mutex>;
template class ACE_Hash_Map_Iterator<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Null_Mutex>;
template class ACE_Hash_Map_Iterator_Ex<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Hash<ACE_HANDLE>, ACE_Equal_To<ACE_HANDLE>, ACE_Null_Mutex>;
template class ACE_Hash_Map_Reverse_Iterator<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Null_Mutex>;
template class ACE_Hash_Map_Reverse_Iterator_Ex<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Hash<ACE_HANDLE>, ACE_Equal_To<ACE_HANDLE>, ACE_Null_Mutex>;

template class ACE_Unbounded_Set<ACE_INET_Addr>;
template class ACE_Unbounded_Set_Iterator<ACE_INET_Addr>;

#elif defined(ACE_HAS_TEMPLATE_INSTANTIATION_PRAGMA)

#pragma instantiate ACE_Hash_Map_Manager<ACE_INET_Addr,ACE_SOCK_Dgram_Mcast*,ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Manager_Ex<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast *, ACE_Hash<ACE_INET_Addr>, ACE_Equal_To<ACE_INET_Addr>, ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Entry<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast *>
#pragma instantiate ACE_Hash<ACE_INET_Addr>
#pragma instantiate ACE_Node<ACE_INET_Addr>
#pragma instantiate ACE_Equal_To<ACE_INET_Addr>
#pragma instantiate ACE_Hash_Map_Iterator_Base_Ex<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast *, ACE_Hash<ACE_INET_Addr>, ACE_Equal_To<ACE_INET_Addr>, ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Iterator<ACE_INET_Addr,ACE_SOCK_Dgram_Mcast*,ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Iterator_Ex<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast*, ACE_Hash<ACE_INET_Addr>, ACE_Equal_To<ACE_INET_Addr>, ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Reverse_Iterator<ACE_INET_Addr,ACE_SOCK_Dgram_Mcast*,ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Reverse_Iterator_Ex<ACE_INET_Addr, ACE_SOCK_Dgram_Mcast*, ACE_Hash<ACE_INET_Addr>, ACE_Equal_To<ACE_INET_Addr>, ACE_Null_Mutex>

#pragma instantiate TAO_ECG_Mcast_EH_T<ACE_SYNCH_MUTEX>

#pragma instantiate ACE_Hash_Map_Manager<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Manager_Ex<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Hash<ACE_HANDLE>, ACE_Equal_To<ACE_HANDLE>, ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Entry<ACE_HANDLE, TAO_Mcast_Socket* >
#pragma instantiate ACE_Hash<ACE_HANDLE>
#pragma instantiate ACE_Node<ACE_HANDLE>
#pragma instantiate ACE_Equal_To<ACE_HANDLE>
#pragma instantiate ACE_Hash_Map_Iterator_Base_Ex<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Hash<ACE_HANDLE>, ACE_Equal_To<ACE_HANDLE>, ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Iterator<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Iterator_Ex<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Hash<ACE_HANDLE>, ACE_Equal_To<ACE_HANDLE>, ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Reverse_Iterator<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Null_Mutex>
#pragma instantiate ACE_Hash_Map_Reverse_Iterator_Ex<ACE_HANDLE, TAO_Mcast_Socket*, ACE_Hash<ACE_HANDLE>, ACE_Equal_To<ACE_HANDLE>, ACE_Null_Mutex>

#pragma instantiate ACE_Unbounded_Set<ACE_INET_Addr>
#pragma instantiate ACE_Unbounded_Set_Iterator<ACE_INET_Addr>

#endif /* ACE_HAS_EXPLICIT_TEMPLATE_INSTANTIATION */
