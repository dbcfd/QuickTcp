#pragma once
#include "quicktcp/utilities/Platform.h"

#include <memory>

namespace quicktcp {
namespace utilities {

class BinarySerializer;

/**
 * Interface for providing caching behavior for an object. Object can be saved as a bytestream, or initialized from a bytestream.
 */
//------------------------------------------------------------------------------
class UTILITIES_API ISerializable
{
public:
	ISerializable();
    virtual ~ISerializable();

	/**
     * Write this object to a serializer. 
     * @param serializer Serializer to write information to
     */
    virtual void writeBinary(BinarySerializer& serializer) const = 0;
    /**
     * Read this object's state from a serializer. 
     * @param serializer Serialier to read state form
     * @return True if state was read successfully
     */
    virtual bool readBinary(BinarySerializer& serializer) = 0;
    /**
     * Estimate the size of this object. This allows serializer to minimize allocations for writes.
     * @return Estimated size of object
     */
    virtual size_t estimateSize() const = 0;

private:
    
};

//Inline implementations
//------------------------------------------------------------------------------

}
}
