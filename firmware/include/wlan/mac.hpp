#pragma once

#include <string_view>
#include <string>
#include <format>
#include <array>

#include <cstdint>

namespace lumina
{

class mac : public std::array<uint8_t, 6>
{
public:

    using base = std::array<uint8_t, 6>;

public:

    /**
     * Default constructor for the `mac` class.
     *
     * @return An instance of the `mac` class.
     *
     * @throws None.
     */
    mac()
    :   base{}
    {}


    /**
     * Constructs an `mac` object from a string representation of a MAC address.
     *
     * @param mac The string representation of the MAC address.
     *
     * @return An instance of the `mac` class.
     *
     * @throws None.
     */
    explicit mac(std::string_view mac)
    {
        std::sscanf(mac.data(), "%hhu:%hhu:%hhu:%hhu:%hhu:%hhu", &(*this)[0], &(*this)[1], &(*this)[2], &(*this)[3], &(*this)[4], &(*this)[5]);
    }

    
    /**
     * Constructs an `mac` object from six octets.
     *
     * @param a The first octet of the MAC address.
     * @param b The second octet of the MAC address.
     * @param c The third octet of the MAC address.
     * @param d The fourth octet of the MAC address.
     * @param e The fifth octet of the MAC address.
     * @param f The sixth octet of the MAC address.
     *
     * @return An instance of the `mac` class.
     *
     * @throws None.
     */
    mac(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f)
    :   base{ a, b, c, d, e, f }
    {}


    operator std::string () const
    {
        return std::format("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}", (*this)[0], (*this)[1], (*this)[2], (*this)[3], (*this)[4], (*this)[5]);
    }

};

}
