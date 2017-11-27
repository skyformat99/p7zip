// Stream/LSBFDecoder.h

#ifndef __STREAM_LSBFDECODER_H
#define __STREAM_LSBFDECODER_H

#include "../IStream.h"

namespace NStream {
namespace NLSBF {

const int kNumBigValueBits = 8 * 4;

const int kNumValueBytes = 3;
const int kNumValueBits = 8  * kNumValueBytes;

const UInt32 kMask = (1 << kNumValueBits) - 1;

extern Byte kInvertTable[256];
// the Least Significant Bit of byte is First

template<class TInByte>
class CDecoder
{
  UInt32 m_BitPos;
  UInt32 m_Value;
  UInt32 m_NormalValue;
protected:
  TInByte m_Stream;
public:
  UInt32 NumExtraBytes;
  bool Create(UInt32 bufferSize) { return m_Stream.Create(bufferSize); }
  void SetStream(ISequentialInStream *inStream) { m_Stream.SetStream(inStream); }
  void ReleaseStream() { m_Stream.ReleaseStream(); }
  void Init()
  {
    m_Stream.Init();
    m_BitPos = kNumBigValueBits; 
    m_Value = 0; // FIXED for valgrind : Use of uninitialised value of size in "void Normalize()", "m_Value = (m_Value << 8) | kInvertTable[b];"
    m_NormalValue = 0;
    NumExtraBytes = 0;
  }
  UInt64 GetProcessedSize() const 
    { return m_Stream.GetProcessedSize() - (kNumBigValueBits - m_BitPos) / 8; }
  UInt64 GetProcessedBitsSize() const 
    { return (m_Stream.GetProcessedSize() << 3) - (kNumBigValueBits - m_BitPos); }

  void Normalize()
  {
    for (;m_BitPos >= 8; m_BitPos -= 8)
    {
      Byte b = 0; // FIXED for valgrind
      if (!m_Stream.ReadByte(b))
        NumExtraBytes++;
      m_NormalValue = (b << (kNumBigValueBits - m_BitPos)) | m_NormalValue;
      m_Value = (m_Value << 8) | kInvertTable[b];
    }
  }
  
  UInt32 GetValue(UInt32 numBits)
  {
    Normalize();
    return ((m_Value >> (8 - m_BitPos)) & kMask) >> (kNumValueBits - numBits);
  }

  void MovePos(UInt32 numBits)
  {
    m_BitPos += numBits;
    m_NormalValue >>= numBits;
  }
  
  UInt32 ReadBits(UInt32 numBits)
  {
    Normalize();
    UInt32 res = m_NormalValue & ( (1 << numBits) - 1);
    MovePos(numBits);
    return res;
  }
  
  UInt32 GetBitPosition() const
  {
    return (m_BitPos & 7);
  }
  
};

}}

#endif
