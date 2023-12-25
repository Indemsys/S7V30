#include "compress.h"
#include "compress_io.h"
#include "Memory_manager.h"

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
static int PutFileChar(compress_data_t* strm, int data) 
{
  CHAR b = data;
  return fx_file_write(strm->pData.file,&b,1);
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
static int GetFileChar(compress_data_t* strm) 
{
  CHAR b;
  ULONG actual_size;
  if (fx_file_read(strm->pData.file,&b,1, &actual_size)!=FX_SUCCESS)
  {
    return EOF;
  }
  return(b); 
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
static void FileRewind(compress_data_t* strm) 
{
  fx_file_seek(strm->pData.file, 0); 
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
static T_compress_size GetFilePos(compress_data_t* strm) 
{
  T_compress_size _nCurPos = strm->pData.file->fx_file_current_file_offset;
  return (_nCurPos);
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
static T_compress_size GetFileLen(compress_data_t* strm) 
{
  T_compress_size _nLen = strm->pData.file->fx_file_current_file_size;
  return(_nLen); 
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
static int PutStringChar(compress_data_t* strm, int data) 
{
  return( strm->pData.str.pData[strm->pData.str.nPos++] = data );
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
static int GetStringChar(compress_data_t* strm) 
{
  if ( strm->pData.str.nPos < strm->pData.str.nLen )
    return(strm->pData.str.pData[strm->pData.str.nPos++]);
  else
    return(EOF); 
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
static void StringRewind(compress_data_t* strm) 
{
  strm->pData.str.nPos = 0; 
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
static T_compress_size GetStringPos(compress_data_t* strm) 
{
  return(strm->pData.str.nPos); 
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
static T_compress_size GetStringLen(compress_data_t* strm) 
{
  return(strm->pData.str.nLen); 
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
void cio_Put_char(compress_data_t* strm, int data)
{
  if ( strm->eType == COMPRESS_FILE_TYPE )
  {
    PutFileChar(strm,data);
  }
  else
  {
    PutStringChar(strm,data);
  }
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
int  cio_Get_char(compress_data_t* strm)  
{
  if ( strm->eType == COMPRESS_FILE_TYPE )
  {
    return(GetFileChar(strm));
  }
  else
  {
    return(GetStringChar(strm));
  }

}


/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
void cio_Rewind(compress_data_t* strm) 
{
  if ( strm->eType == COMPRESS_FILE_TYPE )
  {
    FileRewind(strm);
  }
  else
  {
    StringRewind(strm);
  }
}


/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
T_compress_size  cio_Get_pos(compress_data_t* strm)  
{
  if ( strm->eType == COMPRESS_FILE_TYPE )
  {
    return (GetFilePos(strm));
  }
  else
  {
    return (GetStringPos(strm));
  }
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
T_compress_size  cio_Get_len(compress_data_t* strm)  
{
  if ( strm->eType == COMPRESS_FILE_TYPE )
  {
    return (GetFileLen(strm));
  }
  else
  {
    return (GetStringLen(strm));
  }
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
void *cio_malloc(unsigned int size)
{
  void *ptr;
  ptr = App_malloc_pending(size, 10);
  return (ptr);
}

/*-------------------------------------------------------------------------------------------------------------
 
-------------------------------------------------------------------------------------------------------------*/
void cio_free(void *ptr)
{
  App_free(ptr);
}

