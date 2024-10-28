/* notimplementedexception.hpp
PURPOSE:
- For development
*/
#include <iostream>
#include <exception>

class NotImplementedException : public std::exception {
public:
    const char* what() const noexcept override {
        return "Function not yet implemented.";
    }
};