#include <iostream>
#include <thread>
#include <queue>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>

using namespace std;

// Configuration
const int NUM_PRODUCERS = 2;
const int NUM_CONSUMERS = 2;
const int NUM_TRAFFIC_LIGHTS = 10;
const int MEASUREMENTS_PER_HOUR = 12;
const int TOP_N = 3;

// Shared queue and synchronization primitives
queue<tuple<time_t, int, int>> trafficQueue;
mutex queueMutex;
condition_variable queueCV;
bool done = false;

// Function to simulate a producer generating traffic data
void trafficProducer(int producerId) {
    srand(time(0) + producerId); // Seed random generator uniquely
    for (int i = 0; i < MEASUREMENTS_PER_HOUR; ++i) {
        time_t timestamp = time(0);
        int trafficLightId = rand() % NUM_TRAFFIC_LIGHTS + 1;
        int carsPassed = rand() % 50;
        
        {
            lock_guard<mutex> lock(queueMutex);
            trafficQueue.push(make_tuple(timestamp, trafficLightId, carsPassed));
        }
        queueCV.notify_one();
        
        cout << "[Producer " << producerId << "] Generated: (" << timestamp << ", Traffic Light " << trafficLightId << ", Cars: " << carsPassed << ")\n";
        this_thread::sleep_for(chrono::milliseconds(200 + rand() % 300));
    }
}

// Function to simulate a consumer processing traffic data
void trafficConsumer(int consumerId, unordered_map<int, int>& congestionMap) {
    while (true) {
        tuple<time_t, int, int> trafficData;
        {
            unique_lock<mutex> lock(queueMutex);
            queueCV.wait(lock, [] { return !trafficQueue.empty() || done; });
            if (trafficQueue.empty() && done) return;
            
            trafficData = trafficQueue.front();
            trafficQueue.pop();
        }
        
        int trafficLightId = get<1>(trafficData);
        int carsPassed = get<2>(trafficData);
        congestionMap[trafficLightId] += carsPassed;
        
        // Display top congested traffic lights
        vector<pair<int, int>> congestionVector(congestionMap.begin(), congestionMap.end());
        sort(congestionVector.begin(), congestionVector.end(), [](auto &a, auto &b) { return b.second > a.second; });
        
        cout << "[Consumer " << consumerId << "] Top congested lights:\n";
        for (int i = 0; i < min(TOP_N, (int)congestionVector.size()); ++i) {
            cout << "Traffic Light " << congestionVector[i].first << ": " << congestionVector[i].second << " cars\n";
        }
    }
}

int main() {
    unordered_map<int, int> congestionMap; // To store traffic congestion data
    vector<thread> producers, consumers;
    
    // Start producer threads
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back(trafficProducer, i);
    }
    
    // Start consumer threads
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        consumers.emplace_back(trafficConsumer, i, ref(congestionMap));
    }
    
    // Join producer threads
    for (auto& p : producers) {
        p.join();
    }
    
    // Signal consumers to finish
    {
        lock_guard<mutex> lock(queueMutex);
        done = true;
    }
    queueCV.notify_all();
    
    // Join consumer threads
    for (auto& c : consumers) {
        c.join();
    }
    
    // Final result display
    cout << "\nFinal Congestion Report:\n";
    vector<pair<int, int>> finalResults(congestionMap.begin(), congestionMap.end());
    sort(finalResults.begin(), finalResults.end(), [](auto &a, auto &b) { return b.second > a.se
    cond; });
    
    for (int i = 0; i < min(TOP_N, (int)finalResults.size()); ++i) {
        cout << "Traffic Light " << finalResults[i].first << ": " << finalResults[i].second << " cars passed\n";
    }
    
    return 0;
}
