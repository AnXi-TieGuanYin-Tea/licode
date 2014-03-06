#include <cstring>
#include <boost/shared_ptr.hpp>


#include "../../MediaDefinitions.h"
#include "../../logger.h"
#include "RtpPacketQueue.h"
#include "RtpHeader.h"


namespace erizo{

  DEFINE_LOGGER(RtpPacketQueue, "RtpPacketQueue");
  
  // -----------------------------------------------------------------------------
  // RtpPacketQueue::RtpPacketQueue
  //
  // -----------------------------------------------------------------------------
  //
  RtpPacketQueue::RtpPacketQueue()
    : lastNseq(0), lastTs(0)
  {
  }


  // -----------------------------------------------------------------------------
  // RtpPacketQueue::~RtpPacketQueue
  //
  // -----------------------------------------------------------------------------
  //
  RtpPacketQueue::~RtpPacketQueue(void)
  {
    cleanQueue();
  }

  // -----------------------------------------------------------------------------
  // RtpPacketQueue::packetReceived
  //
  // -----------------------------------------------------------------------------
  //
  void RtpPacketQueue::packetReceived(const char *data, int length)
  {
    //channel->packetReceived2(data, length);
    //return;

    const RTPHeader *header = reinterpret_cast<const RTPHeader*>(data);
    uint16_t nseq = header->getSeqNumber();
    uint32_t ts = header->getTimestamp();

    long long int ltsdiff = (long long int)ts - (long long int)lastTs;
    int tsdiff = (int)ltsdiff;
    int nseqdiff = nseq - lastNseq;
    /*
    // nseq sequence cicle test
    if ( abs(nseqdiff) > ( USHRT_MAX - MAX_DIFF ) )
    {
    NOTIFY("Vuelta del NSeq ns=%d last=%d\n", nseq, lastNseq);
    if (nseqdiff > 0)
    nseqdiff-= (USHRT_MAX + 1);
    else
    nseqdiff+= (USHRT_MAX + 1);
    }
    */
    if (abs(tsdiff) > MAX_DIFF_TS || abs(nseqdiff) > MAX_DIFF )
    {
      // new flow, process and clean queue
      //channel->packetReceived2(data, length);
      lastNseq = nseq;
      lastTs = ts;
      cleanQueue();
    }
    else if (nseqdiff > 1)
    {
      // Jump in nseq, enqueue
      enqueuePacket(data, length, nseq);
//      checkQueue();
    }
    else if (nseqdiff == 1)
    {
      // next packet, process
      // channel->packetReceived2(data, length);
      lastNseq = nseq;
      lastTs = ts;
//      checkQueue();
    }
    else if (nseqdiff < 0)
    {
      // old packet, discard?
      // stats?
    }
    else if (nseqdiff == 0)
    {
      //duplicate packet, process (for stats)?
    }
  }

  // -----------------------------------------------------------------------------
  // RtpPacketQueue::enqueuePacket
  //
  // -----------------------------------------------------------------------------
  //
  void
    RtpPacketQueue::enqueuePacket(const char *data, int length, uint16_t nseq)
    {
      boost::shared_ptr<dataPacket> packet(new dataPacket());
      memcpy(packet->data, data, length);
      packet->length = length;
    /*
      unsigned char *buf = new unsigned char[length];
      memcpy(buf, data, length);
      queue.insert(PACKETQUEUE::value_type(nseq, buf));
      lqueue.insert(LENGTHQ::value_type(nseq, length));
      */
      queue.insert(PACKETQUEUE::value_type(nseq,packet.get()));

    }

  // -----------------------------------------------------------------------------
  // RtpPacketQueue::checkQueue
  //
  // -----------------------------------------------------------------------------
  //
  
  void
    RtpPacketQueue::checkQueue(void)
    {
      // Max size reached, send first
      if (queue.size() >= MAX_SIZE)
      {
        //sendFirst();
      }
      // recorrer la cola para ver si hay paquetes que pueden ser enviados
      while (queue.size() > 0)
      {
        if (queue.begin()->first == lastNseq + 1)
        {
          //sendFirst();
        }
        else
        {
          break;
        }
      }
    }

  // -----------------------------------------------------------------------------
  // RtpPacketQueue::cleanQueue
  //
  // -----------------------------------------------------------------------------
  //
  void
    RtpPacketQueue::cleanQueue(void)
    {
      // vaciar el mapa
      while (queue.size() > 0)
      {
        queue.erase(queue.begin());
      }
    }

  // -----------------------------------------------------------------------------
  // RtpPacketQueue::sendFirst
  //
  // -----------------------------------------------------------------------------
  //
  dataPacket *RtpPacketQueue::getFirst(void)
    {
      dataPacket *packet = queue.begin()->second;
      if (packet == NULL){
        return packet;
      }

      const RTPHeader *header = reinterpret_cast<const RTPHeader*>(packet->data);
      lastNseq = queue.begin()->first;
      lastTs = header->getTimestamp();

      //channel->packetReceived2(data, length);
      queue.erase(queue.begin());
      return packet;

    }

    int RtpPacketQueue::getSize(){
      return queue.size();      
    }


} /* namespace erizo */
