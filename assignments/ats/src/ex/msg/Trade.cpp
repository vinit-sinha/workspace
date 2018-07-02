#include "ex/msg/Trade.h"
#include <limits>

namespace ex{ namespace msg{
    std::istream& operator>>(std::istream & in, Trade& obj)
    {
        in.ignore( std::numeric_limits< std::streamsize >::max(), ','); //Ignore Action
        in >> obj.productId;
        in.ignore( std::numeric_limits< std::streamsize >::max(), ','); //Ignore DELIM
        in >> obj.quantity;
        in.ignore( std::numeric_limits< std::streamsize >::max(), ','); //Ignore DELIM
        in >> obj.price;

        return in;
    }

    std::ostream& operator<<(std::ostream & out, const Trade& obj)
    {
        out << obj.action << ","
            << obj.productId << ","
            << obj.quantity << ","
            << obj.price;

        return out;
    }
}}
