#pragma once

#include <queue>
#include <vector>
#include <algorithm>

class WaveManager
{
private:
    std::queue<int> m_waveQueue;
    std::vector<int> m_unlockedWaves;
    int m_currentWave;

public:
    WaveManager();

    void addWave(int wave);
    bool unlockWave(int wave);
    bool startNextWave();
    void completeCurrentWave();
    void reset();

    int getCurrentWave() const;
    bool hasNextWave() const;
    bool isWaveUnlocked(int wave) const;
    bool isQueueEmpty() const;
    const std::vector<int>& getUnlockedWaves() const;
};
