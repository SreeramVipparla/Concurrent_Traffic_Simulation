#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <random>

#include "Street.h"
#include "Intersection.h"
#include "Vehicle.h"

int WaitingVehicles::getSize()
{
    std::lock_guard<std::mutex> lock(_mutex);

    return _vehicles.size();
}

void WaitingVehicles::pushBack(std::shared_ptr<Vehicle> vehicle, std::promise<void> &&promise)
{
    std::lock_guard<std::mutex> lock(_mutex);

    _vehicles.push_back(vehicle);
    _promises.push_back(std::move(promise));
}

void WaitingVehicles::permitEntryToFirstInQueue()
{
    std::lock_guard<std::mutex> lock(_mutex);


    auto firstPromise = _promises.begin();
    auto firstVehicle = _vehicles.begin();

    
    firstPromise->set_value();
    _vehicles.erase(firstVehicle);
    _promises.erase(firstPromise);
}


Intersection::Intersection()
{
    _type = ObjectType::objectIntersection;
    _isBlocked = false;
}

void Intersection::addStreet(std::shared_ptr<Street> street)
{
    _streets.push_back(street);
}

std::vector<std::shared_ptr<Street>> Intersection::queryStreets(std::shared_ptr<Street> incoming)
{
    std::vector<std::shared_ptr<Street>> outgoings;
    for (auto it : _streets)
    {
        if (incoming->getID() != it->getID()) 
        {
            outgoings.push_back(it);
        }
    }

    return outgoings;
}


void Intersection::addVehicleToQueue(std::shared_ptr<Vehicle> vehicle)
{
    std::unique_lock<std::mutex> lck(_mtx);
    std::cout << "Intersection #" << _id << "::addVehicleToQueue: thread id = " << std::this_thread::get_id() << std::endl;
    lck.unlock();

   
    std::promise<void> prmsVehicleAllowedToEnter;
    std::future<void> ftrVehicleAllowedToEnter = prmsVehicleAllowedToEnter.get_future();
    _waitingVehicles.pushBack(vehicle, std::move(prmsVehicleAllowedToEnter));

   
    ftrVehicleAllowedToEnter.wait();
    lck.lock();
    if (_trafficLight.getCurrentPhase() == TrafficLightPhase::red) {
        _trafficLight.waitForGreen();
    }
    std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " is granted entry." << std::endl;
    lck.unlock();

}

void Intersection::vehicleHasLeft(std::shared_ptr<Vehicle> vehicle)
{
    
    this->setIsBlocked(false);
}

void Intersection::setIsBlocked(bool isBlocked)
{
    _isBlocked = isBlocked;
   
}


void Intersection::simulate() 
{
    
    _trafficLight.simulate();
   
    threads.emplace_back(std::thread(&Intersection::processVehicleQueue, this));
}

void Intersection::processVehicleQueue()
{
   
    while (true)
    {
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (_waitingVehicles.getSize() > 0 && !_isBlocked)
        {
            this->setIsBlocked(true);
            _waitingVehicles.permitEntryToFirstInQueue();
        }
    }
}

bool Intersection::trafficLightIsGreen()
{
   
   if (_trafficLight.getCurrentPhase() == TrafficLightPhase::green)
       return true;
   else
       return false;

} 