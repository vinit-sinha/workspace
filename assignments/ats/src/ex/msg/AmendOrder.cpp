#include "ex/msg/AmendOrder.h"
#include <limits>

namespace ex{ namespace msg{
    std::istream& operator>>(std::istream & in, AmendOrder& obj)
    {
        in.ignore( std::numeric_limits< std::streamsize >::max(), ','); //Ignore Action
        in >> obj.orderId;
        in.ignore( std::numeric_limits< std::streamsize >::max(), ','); //Ignore DELIM
        in >> obj.side;
        in.ignore( std::numeric_limits< std::streamsize >::max(), ','); //Ignore DELIM
        in >> obj.quantity;
        in.ignore( std::numeric_limits< std::streamsize >::max(), ','); //Ignore DELIM
        in >> obj.price;

        return in;
    }

    std::ostream& operator<<(std::ostream & out, const AmendOrder& obj)
    {
        out << obj.action << ","
            << obj.orderId << ","
            << obj.side << ","
            << obj.quantity << ","
            << obj.price;

        return out;
    }
}}
