#include "vznetdp/base/dpmessageproto.h"

const int MAX_BUFFER_SIZE = 1024 * 1024;

void PrintDpMessage(const DpMessage *dp_message){
  std::cout << "type \t\t" << (unsigned)dp_message->type << std::endl;
  std::cout << "method \t\t" << dp_message->method << std::endl;
  std::cout << "method size \t" << (unsigned)dp_message->method_size << std::endl;
  std::cout << "id \t\t"  << dp_message->id << std::endl;
  std::cout << "data_ \t\t" << dp_message->data << std::endl;
  std::cout << "data size \t" << dp_message->data_size << std::endl;
}

bool TestWithIncorrectData(const char* buffer, size_t size){

  vznetdp::DpMessageProto dp_message_proto;
  if (!dp_message_proto.ParserFormBuffer(buffer, size)){
    LOG(ERROR) << "Parse the data getting an error";
    return false;
  }
  const DpMessage *dp_message = dp_message_proto.dp_message();
  PrintDpMessage(dp_message);
  return true;
}

bool TestWithZeroData(const char* buffer, size_t size){

  memset((void *)buffer, 0, size * sizeof(char));

  vznetdp::DpMessageProto dp_message_proto;
  if (!dp_message_proto.ParserFormBuffer(buffer, size)){
    LOG(ERROR) << "Parse the data getting an error";
    return false;
  }
  const DpMessage *dp_message = dp_message_proto.dp_message();
  PrintDpMessage(dp_message);
  return true;
}

bool TestWithCorrectData(const char* buffer, size_t size){

  vznetdp::DpMessageProto dp_message_proto;
  if (!dp_message_proto.ParserFormBuffer(buffer, size)){
    LOG(ERROR) << "Parse the data getting an error";
    return false;
  }
  const DpMessage *dp_message = dp_message_proto.dp_message();
  PrintDpMessage(dp_message);
  return true;
}

int main(int argc, char *argv[]){

  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = 0;
  FLAGS_colorlogtostderr = true;

  char *buffer = new char[MAX_BUFFER_SIZE];
  DpMessage dp_msg;
  dp_msg.type = 0;
  dp_msg.method_size = 8;
  dp_msg.method = "1234567";
  dp_msg.id = 120322;
  dp_msg.data_size = 16;
  dp_msg.data = "123456789012345";
  vznetdp::DpMessageProto dp_message_proto;

  //unsigned int buffer_size = dp_message_proto.SerilaizeToBuffer(
  //  &dp_msg, buffer, MAX_BUFFER_SIZE);

  //TestWithIncorrectData(buffer, 1024);
  //TestWithCorrectData(buffer, buffer_size);
  //TestWithZeroData(buffer, 1024);

  return 0;
} 