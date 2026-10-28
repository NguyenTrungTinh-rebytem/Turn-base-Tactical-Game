#include "WaveManager.h"

WaveManager::WaveManager()
    : m_currentWave(0)
{
}

void WaveManager::addWave(int wave)
{
    if (wave <= 0)
        return;

    std::queue<int> tempQueue = m_waveQueue;

    while (!tempQueue.empty())
    {
        if (tempQueue.front() == wave)
            return;

        tempQueue.pop();
    }

    if (wave == m_currentWave)
        return;

    m_waveQueue.push(wave);
}

bool WaveManager::unlockWave(int wave)
{
    if (wave <= 0)
        return false;

    auto it = std::find(
        m_unlockedWaves.begin(),
        m_unlockedWaves.end(),
        wave
    );

    if (it != m_unlockedWaves.end())
        return false;

    m_unlockedWaves.push_back(wave);

    std::sort(
        m_unlockedWaves.begin(),
        m_unlockedWaves.end()
    );

    return true;
}

bool WaveManager::startNextWave()
{
    if (m_waveQueue.empty())
        return false;

    int nextWave = m_waveQueue.front();

    if (!isWaveUnlocked(nextWave))
        return false;

    m_waveQueue.pop();
    m_currentWave = nextWave;

    return true;
}

void WaveManager::completeCurrentWave()
{
    m_currentWave = 0;
}

void WaveManager::reset()
{
    std::queue<int> emptyQueue;
    m_waveQueue.swap(emptyQueue);

    m_unlockedWaves.clear();
    m_currentWave = 0;
}

int WaveManager::getCurrentWave() const
{
    return m_currentWave;
}

bool WaveManager::hasNextWave() const
{
    return !m_waveQueue.empty();
}

bool WaveManager::isWaveUnlocked(int wave) const
{
    return std::find(
        m_unlockedWaves.begin(),
        m_unlockedWaves.end(),
        wave
    ) != m_unlockedWaves.end();
}

bool WaveManager::isQueueEmpty() const
{
    return m_waveQueue.empty();
}

const std::vector<int>& WaveManager::getUnlockedWaves() const
{
    return m_unlockedWaves;
}
