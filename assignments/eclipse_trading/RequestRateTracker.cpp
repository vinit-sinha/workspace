#include <chrono>

#include "Listener.h"

class RequestRateTracker: public Listener {
private:
    template<typename Clock>
    struct Throttle {
        using TimePoint  = typename Clock::time_point;
        using TimeDuration  = typename Clock::duration;

        Throttle(
            std::size_t thSize, 
            const TimeDuration& thInterval
        ): throttleSize( thSize ),
            throttleInterval( thInterval ),
            startOfCurrentTimeWindow( Clock::now() )
            , lastSeenOn( Clock::now() )
            , count(0)
        {}

        void notify() {
           lastSeenOn = Clock::now();
        }

        // Runtime: Theta(1)
        bool isWithinThrottleLimit(const TimePoint& time) const {
            return ( time - lastSeenOn < throttleInterval ) && ( count < throttleSize );
        }

        // Runtime: Theta(1);
        double throttleOpenTimeWindow(const TimePoint& time) const {
            double openInterval = (time - lastSeenOn).count();
            
            return ( openInterval > 0 ? openInterval : 0.0 );
        }
    private:
       std::size_t throttleSize;
       TimeDuration throttleInterval;

       TimePoint startOfCurrentTimeWindow; 
       TimePoint lastSeenOn; 
       std::size_t count;
    };
public:
    RequestRateTracker(
        std::size_t throttleSize,
        std::chrono::seconds throttleInterval
    );

    void OnInsertOrderRequest(
        int id,
        char side,
        double price,
        int quantity
    );
    
    void OnReplaceOrderRequest(
        int oldId, // The existing order to modify
        int newId, // The new order ID to use if the modification succeeds
        int deltaQuantity
    );

    void OnRequestAcknowledged(
        int id
    );

    void OnRequestRejected(
        int id
    );

    void OnOrderFilled(
        int id,
        int quantityFilled
   );

    bool canReceiveNow() const { 
        return throttle.isWithinThrottleLimit( std::chrono::system_clock::now() );
    }

    double howLongReceiveWindowIsOpen() const {
       return throttle.throttleOpenTimeWindow( std::chrono::system_clock::now() );
    }
private:
    Throttle<std::chrono::system_clock> throttle;

};


RequestRateTracker::RequestRateTracker(
    std::size_t throttleSize,
    std::chrono::seconds throttleInterval
) : Listener(),
    throttle( throttleSize, throttleInterval )
{}
    

void RequestRateTracker::OnInsertOrderRequest(
    int id,
    char side,
    double price,
    int quantity
) {
    throttle.notify();
}
    
void RequestRateTracker::OnReplaceOrderRequest(
    int oldId, // The existing order to modify
    int newId, // The new order ID to use if the modification succeeds
    int deltaQuantity
) {
    throttle.notify();
}

void RequestRateTracker::OnRequestAcknowledged(
   int id
) {
    //NoOpe
}

void RequestRateTracker::OnRequestRejected(
    int id
) {
    //NoOpe
}

void RequestRateTracker::OnOrderFilled(
     int id,
     int quantityFilled
) {
    //NoOpe
}


int main() {
    RequestRateTracker tracker(2, std::chrono::seconds(1));
}
