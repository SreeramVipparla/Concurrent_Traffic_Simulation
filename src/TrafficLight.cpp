#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <queue>

#include "TrafficLight.h"
/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    
        std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); });

    T message = std::move(_queue.back());
    _queue.pop_back();
    
    return message;
    
    }

template <typename T>
void MessageQueue<T>::send(T &&message)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

 std::lock_guard<std::mutex> uLock(_mutex);
 _queue.clear( );
  _queue. emplace_back(std::move(message));
 _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
      _messageQueue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    
         while (true) {
  
		

     TrafficLightPhase phase = _messageQueue->receive();
    if (phase == TrafficLightPhase::green) { return; }
  }
    
    }

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 

 threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 


    std::random_device rdmDev;
    std::mt19937 range(rdmDev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(4,6);
    double randomSeconds = dist(range);
    

    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << "Random time: " << randomSeconds << std::endl;

    while (true) {
        {
            stop = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            if (duration.count()/1000 > randomSeconds) {
                
                if (_currentPhase == green) {
                    _currentPhase = red;
                } else {
                    _currentPhase = green;
                }

               
                TrafficLightPhase phase = _currentPhase;
                std::future<void> _ftr = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, _messageQueue, std::move(phase));

                randomSeconds = dist(range);
                start = std::chrono::high_resolution_clock::now();
            }

            
        }

        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

}
