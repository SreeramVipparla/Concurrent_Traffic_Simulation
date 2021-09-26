#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Graphics.h"
#include "Intersection.h"

void Graphics::simulate()
{
    this->loadBackgroundImg();
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        this->drawTrafficObjects();
    }
}

void Graphics::loadBackgroundImg()
{
    _windowName = "Concurrency Traffic Simulation";
    cv::namedWindow(_windowName, cv::WINDOW_NORMAL);

   
    cv::Mat background = cv::imread(_bgFilename);
    _images.push_back(background);        
    _images.push_back(background.clone()); 
    _images.push_back(background.clone());
}

void Graphics::drawTrafficObjects()
{
    _images.at(1) = _images.at(0).clone();
    _images.at(2) = _images.at(0).clone();

    for (auto it : _trafficObjects)
    {
        double posx, posy;
        it->getPosition(posx, posy);

        if (it->getType() == ObjectType::objectIntersection)
        {
           
            std::shared_ptr<Intersection> intersection = std::dynamic_pointer_cast<Intersection>(it);

            cv::Scalar trafficLightColor = intersection->trafficLightIsGreen() == true ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
            cv::circle(_images.at(1), cv::Point2d(posx, posy), 25, trafficLightColor, -1);
        }
        else if (it->getType() == ObjectType::objectVehicle)
        {
            cv::RNG rng(it->getID());
            int b = rng.uniform(0, 255);
            int g = rng.uniform(0, 255);
            int r = sqrt(255*255 - g*g - b*b); 
            cv::Scalar vehicleColor = cv::Scalar(b,g,r);
            cv::circle(_images.at(1), cv::Point2d(posx, posy), 50, vehicleColor, -1);
        }
    }

    float opacity = 0.85;
    cv::addWeighted(_images.at(1), opacity, _images.at(0), 1.0 - opacity, 0, _images.at(2));
    cv::imshow(_windowName, _images.at(2));
    cv::waitKey(33);
}
