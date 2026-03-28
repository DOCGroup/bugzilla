class TAO_ServerRequestInfo_test_method_longs : public TAO_ServerRequestInfo
{
public:
  TAO_ServerRequestInfo_test_method_longs (
      TAO_ServerRequest &_tao_server_request,
      TAO_Object_Adapter::Servant_Upcall *tao_servant_upcall,
      POA_test *tao_impl,
      const CORBA::Long & a,
      CORBA::Long & b
      ACE_ENV_ARG_DECL_WITH_DEFAULTS
    );

  virtual Dynamic::ParameterList * arguments (
      ACE_ENV_SINGLE_ARG_DECL_WITH_DEFAULTS
    )
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual Dynamic::ExceptionList * exceptions (
      ACE_ENV_SINGLE_ARG_DECL_WITH_DEFAULTS
    )
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual CORBA::Any * result (
      ACE_ENV_SINGLE_ARG_DECL_WITH_DEFAULTS
    )
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual char * target_most_derived_interface (
      ACE_ENV_SINGLE_ARG_DECL_WITH_DEFAULTS
    )
    ACE_THROW_SPEC ((CORBA::SystemException));

  virtual CORBA::Boolean target_is_a (
      const char * id
      ACE_ENV_ARG_DECL_WITH_DEFAULTS
    )
    ACE_THROW_SPEC ((CORBA::SystemException));

  void result (CORBA::Long result);

private:
  TAO_ServerRequestInfo_test_method_longs (
      const TAO_ServerRequestInfo_test_method_longs &
    );

  void operator= (
      const TAO_ServerRequestInfo_test_method_longs &
    );

private:
  POA_test *_tao_impl;

  const CORBA::Long & a_;
  CORBA::Long & b_;
  CORBA::Long _result;
};

// TAO_IDL - Generated from
// C:\irfan\5-3-5\TAO\TAO_IDL\be\be_visitor_operation/interceptors_ss.cpp:438

TAO_ServerRequestInfo_test_method_longs::TAO_ServerRequestInfo_test_method_longs (
    TAO_ServerRequest &_tao_server_request,
    TAO_Object_Adapter::Servant_Upcall *_tao_servant_upcall,
    POA_test *tao_impl,
    const CORBA::Long & a,
    CORBA::Long & b
    ACE_ENV_ARG_DECL_NOT_USED
  )
  : TAO_ServerRequestInfo (_tao_server_request, _tao_servant_upcall),
    _tao_impl (tao_impl),
    a_ (a),
    b_ (b)
{}

Dynamic::ParameterList *
TAO_ServerRequestInfo_test_method_longs::arguments (
    ACE_ENV_SINGLE_ARG_DECL
  )
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  // Generate the argument list on demand.
  Dynamic::ParameterList *parameter_list =
    TAO_RequestInfo_Util::make_parameter_list (ACE_ENV_SINGLE_ARG_PARAMETER);
  ACE_CHECK_RETURN (0);

  Dynamic::ParameterList_var safe_parameter_list = parameter_list;
  parameter_list->length (2);
  CORBA::ULong len = 0;

  (*parameter_list)[len].argument <<= a_;
  (*parameter_list)[len].mode = CORBA::PARAM_IN;
  len++;

  (*parameter_list)[len].argument <<= b_;
  (*parameter_list)[len].mode = CORBA::PARAM_INOUT;
  len++;

  return safe_parameter_list._retn ();
}

Dynamic::ExceptionList *
TAO_ServerRequestInfo_test_method_longs::exceptions (
    ACE_ENV_SINGLE_ARG_DECL
  )
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  // Generate the exception list on demand.
  Dynamic::ExceptionList *exception_list =
    TAO_RequestInfo_Util::make_exception_list (ACE_ENV_SINGLE_ARG_PARAMETER);
  ACE_CHECK_RETURN (0);

  return exception_list;
}

CORBA::Any *
TAO_ServerRequestInfo_test_method_longs::result (
    ACE_ENV_SINGLE_ARG_DECL
  )
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  // Generate the result on demand.
  CORBA::Boolean tk_void_any = 0;
  CORBA::Any *result_any =
    TAO_RequestInfo_Util::make_any (tk_void_any ACE_ENV_ARG_PARAMETER);
  ACE_CHECK_RETURN (0);

  CORBA::Any_var safe_result_any = result_any;

  (*result_any) <<= this->_result;

  return safe_result_any._retn ();
}

char *
TAO_ServerRequestInfo_test_method_longs::target_most_derived_interface (
    ACE_ENV_SINGLE_ARG_DECL_NOT_USED
  )
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  return
    CORBA::string_dup (this->_tao_impl->_interface_repository_id ());
}

CORBA::Boolean
TAO_ServerRequestInfo_test_method_longs::target_is_a (
    const char * id
    ACE_ENV_ARG_DECL
  )
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  return this->_tao_impl->_is_a (id ACE_ENV_ARG_PARAMETER);
}

void
TAO_ServerRequestInfo_test_method_longs::result (CORBA::Long result)
{
  // Update the result.
   this->_result = result;
}
