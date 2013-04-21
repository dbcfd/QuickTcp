#include "Utilities/Serializable.h"
#include "Utilities/BinarySerializer.h"
#include "Utilities/ByteStream.h"

namespace quicktcp {
namespace utilities {

//------------------------------------------------------------------------------
Serializable::Serializable()
{
}

//------------------------------------------------------------------------------
Serializable::~Serializable()
{

}

//------------------------------------------------------------------------------
bool Serializable::readFrom(std::shared_ptr<ByteStream> stream)
{
    bool bReturn = (nullptr != stream);
    if(bReturn)
    {
        BinarySerializer serializer(stream->buffer(), stream->size());
        bReturn = readBinary(serializer);
        bReturn = bReturn && (serializer.bytesRead() == stream->size());
    }

    return bReturn;
}

//------------------------------------------------------------------------------
std::shared_ptr<ByteStream> Serializable::writeTo() const
{
    BinarySerializer serializer(estimateSize());

    writeBinary(serializer);

    return std::shared_ptr<ByteStream>(new ByteStream(serializer.transferBuffer(), serializer.size(), true));
}

}
}
