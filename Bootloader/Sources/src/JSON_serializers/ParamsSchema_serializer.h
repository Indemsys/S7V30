#ifndef PARAMSSCHEMA_SERIALIZER_H
  #define PARAMSSCHEMA_SERIALIZER_H

char*    ParamsSchema_serialize_to_buff(const T_NV_parameters_instance  *p_pars, uint32_t *sz_ptr);
uint32_t ParamsSchema_serialize_to_file(const T_NV_parameters_instance  *p_pars, const char *file_name);


#endif



