#pragma once

#include "ecat_master_future/shm/config.hpp"
#include "ecat_master_future/shm/shared_types.hpp"


// Owns the intermediate serialization buffers 
// and provides drain/push logic shared by both processes.
struct ShmProtoHelper {
    ProtoSlot slot {};

    static bool frame_payload_size(const ProtoSlot& frame, uint32_t& payload_size) {
        if (frame.size < PROTO_FRAME_HEADER_BYTES || frame.size > PROTO_MAX_BYTES)
            return false;

        payload_size =
            static_cast<uint32_t>(frame.data[0]) |
            (static_cast<uint32_t>(frame.data[1]) << 8) |
            (static_cast<uint32_t>(frame.data[2]) << 16) |
            (static_cast<uint32_t>(frame.data[3]) << 24);

        return payload_size > 0 &&
               static_cast<size_t>(payload_size) + PROTO_FRAME_HEADER_BYTES == frame.size;
    }

    // Drain the queue and process every message (NRT usage)
    template<size_t N, typename Proto, typename Fn>
    void drain(SPSCQueue<ProtoSlot, N>& queue, Proto& msg, Fn&& on_msg) {
        while (queue.try_pop(slot)) {
            uint32_t payload_size = 0;
            if (!frame_payload_size(slot, payload_size))
                continue;
            msg.Clear();
            if (msg.ParseFromArray(slot.data + PROTO_FRAME_HEADER_BYTES,
                                   static_cast<int>(payload_size)))
                on_msg(msg);
        }
    }

    // Drain serialized protobuf payloads without deserializing (used for Zenoh)
    template<size_t N, typename Fn>
    void drain_raw(SPSCQueue<ProtoSlot, N>& queue, Fn&& on_frame) {
        while (queue.try_pop(slot)) {
            uint32_t payload_size = 0;
            if (!frame_payload_size(slot, payload_size))
                continue;
            on_frame(
                slot.data + PROTO_FRAME_HEADER_BYTES,
                static_cast<size_t>(payload_size)
            );
        }
    }

    // Drain the queue but keep only latest (for RT usage)
    template<size_t N, typename Proto>
    bool parse_latest(SPSCQueue<ProtoSlot, N>& queue, Proto& msg) {
        bool parsed = false;
        Proto candidate;
        while (queue.try_pop(slot)) {
            uint32_t payload_size = 0;
            if (!frame_payload_size(slot, payload_size))
                continue;

            candidate.Clear();
            if (candidate.ParseFromArray(slot.data + PROTO_FRAME_HEADER_BYTES,
                                         static_cast<int>(payload_size))) {
                msg = candidate;
                parsed = true;
            }
        }
        return parsed;
    }

    // Drain the queue but keep only the latest framed protobuf bytes.
    template<size_t N>
    bool pop_latest_frame(SPSCQueue<ProtoSlot, N>& queue, ProtoSlot& frame) {
        bool popped = false;
        while (queue.try_pop(slot)) {
            uint32_t payload_size = 0;
            if (!frame_payload_size(slot, payload_size))
                continue;

            frame = slot;
            popped = true;
        }
        return popped;
    }

    template<size_t N, typename Proto>
    bool push(SPSCQueue<ProtoSlot, N>& queue, const Proto& msg) {
        // Ask Protobuf how many bytes the message will occupy once serialized
        const size_t payload_size = msg.ByteSizeLong();
        if (payload_size == 0 || payload_size + PROTO_FRAME_HEADER_BYTES > PROTO_MAX_BYTES)
            return false;

        const uint32_t payload_u32 = static_cast<uint32_t>(payload_size);
        slot.data[0] = static_cast<uint8_t>(payload_u32 & 0xFF);
        slot.data[1] = static_cast<uint8_t>((payload_u32 >> 8) & 0xFF);
        slot.data[2] = static_cast<uint8_t>((payload_u32 >> 16) & 0xFF);
        slot.data[3] = static_cast<uint8_t>((payload_u32 >> 24) & 0xFF);

        // Serialize the protobuf payload after the 4-byte little-endian length prefix.
        if (!msg.SerializeToArray(slot.data + PROTO_FRAME_HEADER_BYTES,
                                  static_cast<int>(payload_size)))
            return false;

        slot.size = static_cast<uint32_t>(payload_size + PROTO_FRAME_HEADER_BYTES);
        return queue.try_push(slot);
    }

    template<size_t N, typename Proto, typename Fn>
    void peek_all(const SPSCQueue<ProtoSlot, N>& queue,
                Proto& msg,
                Fn&& on_msg)
    {
        const size_t count = queue.size();

        for (size_t i = 0; i < count; ++i)
        {
            ProtoSlot frame;

            if (!queue.peek(i, frame))
                continue;

            uint32_t payload_size = 0;

            if (!frame_payload_size(frame, payload_size))
                continue;

            msg.Clear();

            if (msg.ParseFromArray(
                    frame.data + PROTO_FRAME_HEADER_BYTES,
                    static_cast<int>(payload_size)))
            {
                on_msg(msg);
            }
        }
    }

    template<size_t N, typename Proto>
    bool peek_latest(const SPSCQueue<ProtoSlot, N>& queue, Proto& msg)
    {
        const size_t count = queue.size();

        if (count == 0)
            return false;

        ProtoSlot frame;

        if (!queue.peek(count - 1, frame))
            return false;

        uint32_t payload_size = 0;

        if (!frame_payload_size(frame, payload_size))
            return false;

        msg.Clear();

        return msg.ParseFromArray(
            frame.data + PROTO_FRAME_HEADER_BYTES,
            static_cast<int>(payload_size));
    }
};