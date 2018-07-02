#pragma once
#include "ex/type/Types.h"
#include "ex/msg/NewOrder.h"
#include "ex/msg/AmendOrder.h"
#include "ex/msg/CancelOrder.h"
#include "ex/msg/Trade.h"

namespace ex{ namespace msg{
    template<typename OnDecode> struct Decoder {
        Decoder(std::istream& i, OnDecode& onDecodeHandler)
            : in(i)
              , onDecode(onDecodeHandler)
        {}

        Decoder() = delete;
        Decoder(const Decoder&) = delete;
        Decoder(Decoder&&) = delete;
        Decoder& operator=(const Decoder&) = delete;
        Decoder& operator=(Decoder&&) = delete;


        bool hasMoreMessages() const 
        {
            return !in.eof() && !in.bad();
        }

        void decode() 
        {
            ex::type::Action action = decodeAction();
            switch (action) {
                case ex::type::Action::New:
                    decodeMsg<ex::msg::NewOrder>();
                    break;
                case ex::type::Action::Amend:
                    decodeMsg<ex::msg::AmendOrder>();
                    break;
                case ex::type::Action::Cancel:
                    decodeMsg<ex::msg::CancelOrder>();
                    break;
                case ex::type::Action::Trade:
                    decodeMsg<ex::msg::Trade>();
                    break;
                default:
                   break; 
            }
        }
    private:
        std::istream& in;
        OnDecode& onDecode;

        ex::type::Action decodeAction() 
        {
            ex::type::Action action = ex::type::Action::Unknown;
            in >> action;
            return action;
        }

        template<typename Msg> void decodeMsg() 
        {
            Msg m;
            in >> m;
            onDecode(const_cast<const Msg&>(m));
        }
    };

}}
