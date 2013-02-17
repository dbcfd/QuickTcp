#pragma once
#include "utilities/Platform.h"
#include "utilities/ISerializable.h"

#include <memory>

namespace quicktcp {
namespace utilities {

class ByteStream;

/**
 * Interface for providing caching behavior for an object. Object can be saved as a bytestream, or initialized from a bytestream.
 */
//------------------------------------------------------------------------------
class CACHE_API Serializable : protected ISerializable
{
public:
    Serializable();
    virtual ~Serializable();

    /**
     * Initialize object from byte stream. 
     * @return True if initialization was successful.
     */
    bool ReadFrom(std::shared_ptr<ByteStream> stream);
    /**
     * Save object state to a bytestream
     * @return Shared pointer to bytestream representing this object's state
     */
    std::shared_ptr<ByteStream> WriteTo() const;
	
private:
    
};

//Inline implementations
//------------------------------------------------------------------------------

}
}
