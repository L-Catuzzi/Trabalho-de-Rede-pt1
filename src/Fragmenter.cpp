std::vector<SLOWPacket> Fragmenter::fragmentPayload(
    const std::array<uint8_t, 16>& sid,
    uint32_t sttl,
    uint32_t& seqStart,
    uint16_t window,
    const std::vector<uint8_t>& payload) {

    const size_t max_data = 1440;
    uint8_t fid = static_cast<uint8_t>(rand() % 255);
    std::vector<SLOWPacket> fragments;

    for (size_t offset = 0, fo = 0; offset < payload.size(); ++fo) {
        size_t chunk = std::min(max_data, payload.size() - offset);

        SLOWPacket pkt;
        pkt.sid = sid;
        pkt.sttl = sttl;
        pkt.flags = SLOWFlags::ACK | SLOWFlags::MB;
        pkt.seqnum = seqStart++;
        pkt.acknum = 0;
        pkt.window = window;
        pkt.fid = fid;
        pkt.fo = static_cast<uint8_t>(fo);
        pkt.data.insert(pkt.data.begin(),
                        payload.begin() + offset,
                        payload.begin() + offset + chunk);

        offset += chunk;
        if(offset >= payload.size()) pkt.flags &= ~SLOWFlags::MB;

        fragments.push_back(pkt);
    }

    return fragments;
}