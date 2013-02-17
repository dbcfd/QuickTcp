#include "utilities/Serializable.h"
#include "utilities/BinarySerializer.h"
#include "utilities/ByteStream.h"

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
bool Serializable::ReadFrom(std::shared_ptr<ByteStream> stream)
{
    bool bReturn = (nullptr != stream);
    if(bReturn)
    {
        BinarySerializer serializer(stream->Buffer(), stream->Size());
        bReturn = ReadBinary(serializer);
        bReturn = bReturn && (serializer.BytesRead() == stream->Size());
    }

    return bReturn;
}

//------------------------------------------------------------------------------
std::shared_ptr<ByteStream> Serializable::WriteTo() const
{
    BinarySerializer serializer(EstimateSize());

    WriteBinary(serializer);

    return std::shared_ptr<ByteStream>(new ByteStream(serializer.TransferBuffer(), serializer.Size(), true));
}

}
}
