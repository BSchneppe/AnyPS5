#include "prx/libSceAgc/Command/include/Control.hpp"

namespace Agc::Command {

std::uint32_t* WriteJump(CommandBuffer* buffer, std::uint8_t chain, std::uint8_t cachePolicy, const std::uint32_t* target, std::uint32_t sizeInDwords, const char* function) {
    const auto guestAddress = reinterpret_cast<std::uintptr_t>(target);
    CheckBits(chain, 1, function);
    CheckBits(cachePolicy, 3, function);
    CheckBits(sizeInDwords, 0xfffffu, function);
    Require((guestAddress & 3u) == 0, function, "misaligned jump target");
    return Emit(buffer, 0x3fu, {static_cast<std::uint32_t>(guestAddress) & ~3u, static_cast<std::uint32_t>(guestAddress >> 32u), 0x0f200000u | (static_cast<std::uint32_t>(cachePolicy) << 28u) | (static_cast<std::uint32_t>(chain) << 20u) | sizeInDwords}, function);
}

std::uint32_t* WriteRewind(CommandBuffer* buffer, std::uint32_t initialState, const char* function) {
    return Emit(buffer, 0x59u, {(initialState & 1u) << 31u}, function);
}

std::uint32_t* WritePredication(CommandBuffer* buffer, std::uint8_t condition, std::uint8_t operation, std::uint8_t waitOperation, const volatile void* address, const char* function) {
    CheckBits(condition, 1, function);
    CheckBits(operation, 7, function);
    CheckBits(waitOperation, 1, function);
    const auto guestAddress = reinterpret_cast<std::uintptr_t>(address);
    if (operation != 0) {
        CheckAddress(guestAddress, 8, function);
    }
    return Emit(buffer, 0x20u, {(static_cast<std::uint32_t>(condition) << 8u) | (static_cast<std::uint32_t>(waitOperation) << 12u) | (static_cast<std::uint32_t>(operation) << 16u), static_cast<std::uint32_t>(guestAddress) & ~0xfu, static_cast<std::uint32_t>(guestAddress >> 32u)}, function);
}

std::uint32_t* WriteMemSemaphore(CommandBuffer* buffer, const volatile void* address, std::uint8_t waitForMailbox, std::uint8_t signalType, std::uint8_t operation, const char* function) {
    const auto guestAddress = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(address));
    const auto maskedAddress = guestAddress & ~0xfULL;
    auto* packet = Allocate(buffer, 8, function);
    packet[0] = Header(0x79u, 4);
    packet[1] = 0x342u;
    packet[2] = 0xc9020000u | static_cast<std::uint32_t>((guestAddress >> 32u) & 0xffffu);
    packet[3] = static_cast<std::uint32_t>(guestAddress);
    packet[4] = Header(0x39u, 4);
    packet[5] = static_cast<std::uint32_t>(maskedAddress);
    packet[6] = static_cast<std::uint32_t>(maskedAddress >> 32u);
    packet[7] = ((static_cast<std::uint32_t>(operation) & 7u) << 29u) | ((static_cast<std::uint32_t>(signalType) & 1u) << 20u) | ((static_cast<std::uint32_t>(waitForMailbox) & 1u) << 16u);
    return packet;
}

}
