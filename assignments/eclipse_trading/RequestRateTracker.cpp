#include <chrono>
#include <iostream>

#include "Listener.h"

class RequestRateTracker: public Listener {
private:
    // An utility class which can check the rate of message arrival
    //  and inform whenever message arrival rate exceeds the specifed rate.
    //  throtte limit is specifed by number of messages (throttleSize) and
    //  interval(throttleInterval) during which that many messages are alllowed
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
            , lastSeenOn( startOfCurrentTimeWindow )
            , count(0)
        {}

        void notify() {
           lastSeenOn = Clock::now();
           count++;
           if( lastSeenOn > startOfCurrentTimeWindow + throttleInterval )
                reset();
        }

        void reset() {
            startOfCurrentTimeWindow = Clock::now();
            lastSeenOn = startOfCurrentTimeWindow;
            count = 0;
        }

        // Runtime: Theta(1)
        bool isWithinThrottleLimit(const TimePoint& time) const {
            return ( time - lastSeenOn < throttleInterval ) && ( count < throttleSize );
        }

        // Runtime: Theta(1);
        double waitPeriod(const TimePoint& time) const {
            double waitInterval = (time - lastSeenOn).count();
            
            return ( isWithinThrottleLimit(time) ?  0.0 : waitInterval );
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

    bool hasThrottleHit() const { 
        return ! throttle.isWithinThrottleLimit( std::chrono::system_clock::now() );
    }

    double howLongToWait() const {
        return throttle.waitPeriod( std::chrono::system_clock::now() );
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

// test program
int main() {
    RequestRateTracker tracker(2, std::chrono::seconds(1));
    if( tracker.hasThrottleHit() ) {
        std::cout <<"Test Case Failed at " <<  __LINE__ << std::endl;
        return -1;
    }

    if( tracker.howLongToWait() != 0.0 ) {
        std::cout <<"Test Case Failed at " <<  __LINE__ << " due to " <<  tracker.howLongToWait() << std::endl;
        return -1;
    }

    tracker.OnInsertOrderRequest(1, 'B', 10, 10.0 );

    if( tracker.hasThrottleHit() ) {
        std::cout <<"Test Case Failed at " <<  __LINE__ << std::endl;
        return -1;
    }

    if( tracker.howLongToWait() != 0.0 ) {
        std::cout <<"Test Case Failed at " <<  __LINE__ << " due to " <<  tracker.howLongToWait() << std::endl;
        return -1;
    }
    tracker.OnInsertOrderRequest(2, 'B', 10, 10.0 );
    
    if( ! tracker.hasThrottleHit() ) {
        std::cout <<"Test Case Failed at " <<  __LINE__ << std::endl;
        return -1;
    }

    if( tracker.howLongToWait() == 0.0 ) {
        std::cout <<"Test Case Failed at " <<  __LINE__ << " due to " <<  tracker.howLongToWait() << std::endl;
        return -1;
    }

    std::cout << "Tests Passed!" << std::endl;
}
