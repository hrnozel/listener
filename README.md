# listener
listener is a header only multicast listener library. It is developed in C++ and supports Cross-platform.

It uses the posix socket api.

example:

```
  #include "listener.hpp"
  
  int main(int argc, char** argv)
  {
    MulticastListener listener{"224.1.1.5", 8201};
    
    if (!listener.listen())
    {
      std::cout << "Multicast listener could not listen" << '\n';
    }
    
    std::string msg{};
    do
    {
      msg = listener.getMessage();
    } while(msg.empty());
    
    std::cout << "Your multicast message is: " << msg << std::endl;
    
    return 0;
  }
  
  ```
