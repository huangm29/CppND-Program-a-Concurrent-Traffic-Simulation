#include "TrafficLight.h"
#include <iostream>
#include <random>

/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {
  // FP.5a : The method receive should use std::unique_lock<std::mutex> and
  // _condition.wait() to wait for and receive new messages and pull them from
  // the queue using move semantics. The received object should then be returned
  // by the receive function.
  std::unique_lock<std::mutex> lock(_mutex);
  // Wait until the condition is satisfied, i.e., the queue is not empty
  _condition.wait(lock, [this]() { return !_queue.empty(); });
  T item = std::move(_queue.front());
  // _queue.pop_front();
  _queue.clear(); // More efficient
  return item;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
  // FP.4a : The method send should use the mechanisms
  // as well as _condition.notify_one() to add a new message to the queue and
  // afterwards send a notification.

  std::lock_guard<std::mutex> lock(_mutex);
  _queue.emplace_back(std::move(msg));
  _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  // FP.5b : add the implementation of the method waitForGreen, in which an
  // infinite while-loop runs and repeatedly calls the receive function on the
  // message queue. Once it receives TrafficLightPhase::green, the method
  // returns.
  while (true) {
    TrafficLightPhase receivedPhase = _messageQueue.receive();
    if (receivedPhase == TrafficLightPhase::green) {
      return;
    }
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  // FP.2b : Finally, the private method „cycleThroughPhases“ should be
  // started in a thread when the public method „simulate“ is called. To do
  // this, use the thread queue in the base class.
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // FP.2a : Implement the function with an infinite loop that measures the
  // time between two loop cycles and toggles the current phase of the traffic
  // light between red and green and sends an update method to the message
  // queue using move semantics. The cycle duration should be a random value
  // between 4 and 6 seconds. Also, the while-loop should use
  // std::this_thread::sleep_for to wait 1ms between two cycles.
  // Create a random device for generating random numbers
  std::random_device rd;

  // Create a random number generator and seed it with the random device
  std::mt19937 gen(rd());

  // Define a uniform distribution with a range between 4000 and 6000
  std::uniform_int_distribution<> distr(4000, 6000);

  while (true) {
    auto start = std::chrono::high_resolution_clock::now();
    // Toggle the traffic light phase between red and green
    _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green
                                            : TrafficLightPhase::red;
    double cycleDuration = distr(gen);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    if (elapsed.count() < cycleDuration) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(static_cast<long long>(cycleDuration) -
                                    static_cast<long long>(elapsed.count())));
      _messageQueue.send(std::move(_currentPhase));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
