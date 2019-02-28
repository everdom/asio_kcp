#include <iostream>
#include <memory.h>
#include "kcp_buffer_data.hpp"


namespace asio_kcp {

  kcp_buffer_data::kcp_buffer_data():
    buffer_size(0)
  {
  }

  kcp_buffer_data::~kcp_buffer_data(){
    
  }


  kcp_buffer_data::kcp_buffer_data(const char *data, long size):
    buffer_size(0)
  {
    set_data(data, size);
  }

  int kcp_buffer_data::set_data(const char *data, long size){
    if(size < 0){
      std::cerr << "kcp_buffer_data::set_data size < 0" << std::endl;
      return -1;
    }

    if(size > MAX_DATA_SIZE){
      std::cerr << "kcp_buffer_data::set_data size "<<size<<"> MAX_DATA_SIZE("<<MAX_DATA_SIZE<<")" << std::endl;
      return -2;
    }

    if(memcpy(buffer_data, data, size) == NULL){
      std::cerr << "kcp_buffer_data::set_data memcpy error"<< std::endl;
      return -3;
    }

    buffer_size = size;

    return 0;
  }

  char *kcp_buffer_data::data(){
    return buffer_data;
  }

  long kcp_buffer_data::size(){
    return buffer_size;
  }

}
