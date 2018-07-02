#include "ex/type/Types.h"

namespace ex{ namespace type{
    std::ostream & operator<<(std::ostream & out, Action action)
    {
        out << static_cast<char>(action);
        return out;
    }

    std::istream & operator>>(std::istream & in, Action& action)
    {
        char a;
        in >> a;
        action = static_cast<Action>(a);
        switch(action) {
            case Action::New:
                break;
            case Action::Amend:
                break;
            case Action::Cancel:
                break;
            case Action::Trade:
                break;
            default:
                action = Action::Unknown;
        }
        return in;
    }

    std::ostream & operator<<(std::ostream & out, Side side)
    {
        out << static_cast<char>(side);
        return out;
    }

    std::istream & operator>>(std::istream & in, Side& side)
    {
        char s;
        in >> s;
        side = static_cast<Side>(s);
        switch(side) {
            case Side::Buy:
                break;
            case Side::Sell:
                break;
            default:
                side = Side::Unknown;
        }
        return in;
    }

    std::ostream & operator<<(std::ostream & out,const ErrorCode& e)
    {
        switch(e) {
            case ErrorCode::Ok:
                out <<"No Error"; break;
            case ErrorCode::InvalidOrderId:
                out <<"Invalid OrderID Error"; break;
            case ErrorCode::InvalidProductId:
                out <<"Invalid ProductID Error"; break;
            case ErrorCode::DuplicateOrderId:
                out <<"Duplicate OrderID Error"; break;
            case ErrorCode::DuplicateProductId:
                out <<"Duplicate ProductID Error"; break;
            case ErrorCode::InvalidPrice:
                out <<"Invalid Price Error"; break;
            case ErrorCode::InvalidQuantity:
                out <<"Invalid Quantity Error"; break;
            case ErrorCode::CorruptMessage:
                out <<"Corrupt Message Received Error"; break;
            case ErrorCode::Unknown:
            default:
                out <<"Unknown Error"; break;
        }

        return out;
    }
}}
