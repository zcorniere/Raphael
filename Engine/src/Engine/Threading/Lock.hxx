#pragma once

#include <atomic>
#include <emmintrin.h>

class FRWFifoLock
{
public:
    void ReadLock()
    {
        std::unique_lock<std::recursive_mutex> lock(m_Mutex);    // Lock the mutex in case a writer is on it
        ++m_readers;
    }

    void WriteLock()
    {
        m_Mutex.lock();    // Lock the mutex to prevent readers from entering,

        // Wait for all readers to finish, yeilding the CPU
        for (;;)
        {
            if (m_readers.load() == 0)
            {
                break;
            }
            _mm_pause();
        }
    }

    void ReadUnlock()
    {
        --m_readers;
    }

    void WriteUnlock()
    {
        m_Mutex.unlock();
    }

private:
    std::recursive_mutex m_Mutex;
    std::atomic_int32_t m_readers = 0;
};
