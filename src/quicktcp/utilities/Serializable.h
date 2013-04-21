#pragma once
#include "Utilities/Platform.h"
#include "Utilities/ISerializable.h"

#include <memory>

namespace quicktcp {
namespace utilities {

class ByteStream;

/**
 * Interface for providing caching behavior for an object. Object can be saved as a bytestream, or initialized from a bytestream.
 */
//------------------------------------------------------------------------------
class UTILITIES_API Serializable : protected ISerializable
{
public:
    Serializable();
    virtual ~Serializable();

    /**
     * Initialize object from byte stream. 
     * @return True if initialization was successful.
     */
    bool readFrom(std::shared_ptr<ByteStream> stream);
    /**
     * Save object state to a bytestream
     * @return Shared pointer to bytestream representing this object's state
     */
    std::shared_ptr<ByteStream> writeTo() const;
	
private:
    
};

//Inline implementations
//------------------------------------------------------------------------------

}
}
