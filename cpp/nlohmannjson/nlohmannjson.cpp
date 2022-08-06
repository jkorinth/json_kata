#include "3rdparty/single_include/nlohmann/json.hpp"
#include <iostream>

using json = nlohmann::json;

int
main(int argc, char* argv[])
{
  json data = json::parse(std::cin);
  std::cout << data.dump() << std::endl;
  return 0;
}
