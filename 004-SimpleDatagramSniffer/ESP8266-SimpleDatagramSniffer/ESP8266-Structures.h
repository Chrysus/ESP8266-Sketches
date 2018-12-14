/*
  The structures in this file are derrived from information in the ESP8266 documentation
  found here:

    https://www.espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf

  * See Chapter 14
 */

struct PacketData {
  signed rssi            : 8;         // signal intensity of packet
  unsigned rate          : 4;
  unsigned is_group      : 1;
  unsigned               : 1;
  unsigned sig_mode      : 2;         // 0b00 = this is not a 802.11n packet; otherwise this is a 802.11n packet
  unsigned legacy_length :12;         // if this is not a 802.11n packet, shows the length of the packet
  unsigned damatch0      : 1;
  unsigned damatch1      : 1;
  unsigned bssidmatch0   : 1;
  unsigned bssidmatch1   : 1;
  unsigned MCS           : 7;         // if this is a 802.11n packet, shows the modulation and code used (range from 0 to 76)
  unsigned CWB           : 1;         // if is a 802.11n packet, shows whether or not this is a HT40 packet
  unsigned HT_length     :16;         // if is a 802.11n packet, shows the length of packet
  unsigned Smoothing     : 1;
  unsigned Not_Sounding  : 1;
  unsigned               : 1;
  unsigned Aggregation   : 1;
  unsigned STBC          : 2;
  unsigned FEC_CODING    : 1;         // if is a 802.11n packet, shows whether or not this is a LDPC packet.
  unsigned SGI           : 1;
  unsigned rxend_state   : 8;
  unsigned ampdu_cnt     : 8;
  unsigned channel       : 4;
  unsigned               :12;
};

// IEEE80211 packet header

struct PacketHeader {
  byte frame_control[2];
  byte duration[2];
  byte mac_address_1[6];
  byte mac_address_2[6];
  byte mac_address_3[6];
  byte seq_ctl[2];
  byte mac_address_4[6];
  byte qos_control[2];
  byte HT_control[4];
};

struct PromiscuousDataSmall {
  struct PacketData packet_data;
  struct PacketHeader packet_header;
  uint16_t count;                     // number count of packet
  uint16_t len;                       // length of the packet
  uint16_t seq;                       // serial number of the packet, the high 12 bits are the serial number,
                                      // the low 14 bits are fragment number (this will generally be 0)
  uint8_t mac_address_3[6];
};

struct PromiscuousDataLarge {
  struct PacketData packet_data;
  uint8_t buf[112];                    // this may be 240, please refer to the real source code
  uint16_t count;                      // number count of packet
  uint16_t len;                        // length of packet
};



/*

https://www.espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf


  Callback wifi_promiscuous_rx has two parameters (buf and len). len means the
length of buf, it can be: len = sizeof(struct sniffer_buf2), len = X * 10, len = sizeof(struct
RxControl):
Case of LEN == sizeof (struct sniffer_buf2)
• buf contains structure sniffer_buf2: it is the management packet, it has 112
Bytes data.
• sniffer_buf2.cnt is 1.
• sniffer_buf2.len is the length of packet.
Case of LEN == X * 10
• buf contains structure sniffer_buf: this structure is reliable, data packets
represented by it has been verified by CRC.
• sniffer_buf.cnt means the count of packets in buf. The value of len depends
on sniffer_buf.cnt.
- sniffer_buf.cnt==0, invalid buf; otherwise, len = 50 + cnt * 10
• sniffer_buf.buf contains the first 36 Bytes of IEEE80211 packet. Starting from
sniffer_buf.lenseq[0], each structure lenseq represent a length information of
packet. lenseq[0] represents the length of first packet. If there are two packets
where (sniffer_buf.cnt == 2), lenseq[1] represents the length of second
packet.
• If sniffer_buf.cnt > 1, it is a AMPDU packet, head of each MPDU packets are
similar, so we only provide the length of each packet (from head of MAC packet to
FCS)
• This structure contains: length of packet, MAC address of both sides of
communication, length of the head of packet.
Case of LEN == sizeof(struct RxControl)
• buf contains structure RxControl; but this structure is not reliable, we can not get
neither MAC address of both sides of communication nor length of the head of
packet.
• For AMPDU packet, we can not get the count of packets or the length of packet. 
This structure contains: length of packet, rssi and FEC_CODING.
• RSSI and FEC_CODING are used to guess if the packets are sent from same device.




For the case of LEN == sizeof(struct RxControl), the methods to calculate the length of packet are as below:
• If sig_mode == 0, the length of packet is the legacy_length.
• Otherwise, the length of packet is in struct sniffer_buf and sniffer_buf2, and it is more reliable

*/
