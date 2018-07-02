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
}}
