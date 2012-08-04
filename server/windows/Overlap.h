#pragma once

#include <sstream>

#include "server/windows/Platform.h"
#include "server/windows/Winsock2.h"

namespace quicktcp {
namespace server {
namespace windows {

/**
 * Overlaps are used by Winsock2 asynchronous calls to provide notifications when events occur,
 * such as a send or recv completing, or a client attempting to connect. Overlaps must be
 * created in a specific way that not only allocates memory appropriately, but also prepares
 * event handles to check for status by WaitForMultipleEvents. To fully describe connections
 * associated with overlaps, the overlap structure must be extended.
 */
template<class T>
class Overlap
{
public:
    template<class T>
    struct OverlapExtend: public WSAOVERLAPPED
    {
        T* derived;
    };
    typedef OverlapExtend<T> OverlapType;
    Overlap()
            : mOverlap(nullptr)
    {
    }
    Overlap(T* derived)
    {
        createOverlap(derived);
    }
    virtual ~Overlap()
    {
        WSACloseEvent(mOverlap->hEvent);
        mOverlap->hEvent = WSA_INVALID_EVENT;

        if (mOverlap != nullptr)
        {
            HeapFree(GetProcessHeap(), 0, mOverlap);
            mOverlap = nullptr;
        }
    }

    inline OverlapType* getOverlap() const
    {
        return mOverlap;
    }

protected:
    void createOverlap(T* derived)
    {
        //
        // Allocate an overlapped structure.
        // We use the Offset field to keep track of the socket handle
        // we have accepted a connection on, since there is no other
        // way to pass information to GetOverlappedResult()
        //
        OverlapType* cOverlap = (OverlapType*) HeapAlloc(GetProcessHeap(),
                HEAP_ZERO_MEMORY, sizeof(OverlapType));

        //
        // Did the HeapAllocation FAIL??
        //
        if (cOverlap == nullptr)
        {
            throw(std::runtime_error("Overlap HeapAlloc()"));
        }

        SecureZeroMemory(cOverlap, sizeof(OverlapType));

        if (WSA_INVALID_EVENT == (cOverlap->hEvent = WSACreateEvent()))
        {
            throw(std::runtime_error("WSACreateEvent"));
        }

        cOverlap->derived = derived;

        mOverlap = cOverlap;
    }
private:
    OverlapType* mOverlap;
};

}
}
}
