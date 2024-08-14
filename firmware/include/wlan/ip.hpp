#pragma once

#include <string_view>
#include <string>
#include <format>
#include <array>

#include <cstdint>

namespace lumina
{

class ipv4 : public std::array<uint8_t, 4>
{
public:

    using base = std::array<uint8_t, 4>;
    
public:

    /**
     * Default constructor for the `ipv4` class.
     *
     * @return An instance of the `ipv4` class.
     * 
     * @throws None.
     */
    ipv4()
    :   base{}
    {}


    /**
     * Constructs an `ipv4` object from a string representation of an IP address.
     *
     * @param ip The string representation of the IP address.
     * 
     * @return An instance of the `ipv4` class.
     *
     * @throws None.
     */
    ipv4(std::string_view ip)
    {
        std::sscanf(ip.data(), "%hhu.%hhu.%hhu.%hhu", &(*this)[0], &(*this)[1], &(*this)[2], &(*this)[3]);
    }

    
    /**
     * Constructs an `ipv4` object from four octets.
     *
     * @param a The first octet of the IP address.
     * @param b The second octet of the IP address.
     * @param c The third octet of the IP address.
     * @param d The fourth octet of the IP address.
     *
     * @return An instance of the `ipv4` class.
     * 
     * @throws None.
     */
    ipv4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    :   base{ a, b, c, d }
    {}

    
    /**
     * Constructs an `ipv4` object from a 32-bit integer representation of an IP address.
     *
     * @param ip The 32-bit integer representation of the IP address.
     *
     * @return An instance of the `ipv4` class.
     *
     * @throws None.
     */
    ipv4(uint32_t ip)
    :   base{
            static_cast<uint8_t>((ip >> 24) & 0xFF),
            static_cast<uint8_t>((ip >> 16) & 0xFF),
            static_cast<uint8_t>((ip >> 8) & 0xFF),
            static_cast<uint8_t>(ip & 0xFF)
        }
    {}


    /**
     * Bitwise AND operator for `ipv4` objects.
     *
     * @param mask The `ipv4` object to perform the bitwise AND operation with.
     *
     * @return A new `ipv4` object containing the result of the bitwise AND operation.
     *
     * @throws None.
     */
    ipv4 operator& (ipv4 mask) const
    {
        return {
            static_cast<uint8_t>((*this)[0] & mask[0]),
            static_cast<uint8_t>((*this)[1] & mask[1]),
            static_cast<uint8_t>((*this)[2] & mask[2]),
            static_cast<uint8_t>((*this)[3] & mask[3])
        };
    }


    /**
     * Bitwise OR operator for `ipv4` objects.
     *
     * @param mask The `ipv4` object to perform the bitwise OR operation with.
     *
     * @return A new `ipv4` object containing the result of the bitwise OR operation.
     *
     * @throws None.
     */
    ipv4 operator| (ipv4 mask) const
    {
        return {
            static_cast<uint8_t>((*this)[0] | mask[0]),
            static_cast<uint8_t>((*this)[1] | mask[1]),
            static_cast<uint8_t>((*this)[2] | mask[2]),
            static_cast<uint8_t>((*this)[3] | mask[3])
        };
    }


    /**
     * Bitwise XOR operator for `ipv4` objects.
     *
     * @param mask The `ipv4` object to perform the bitwise XOR operation with.
     *
     * @return A new `ipv4` object containing the result of the bitwise XOR operation.
     *
     * @throws None.
     */
    ipv4 operator^ (ipv4 mask) const
    {
        return {
            static_cast<uint8_t>((*this)[0] ^ mask[0]),
            static_cast<uint8_t>((*this)[1] ^ mask[1]),
            static_cast<uint8_t>((*this)[2] ^ mask[2]),
            static_cast<uint8_t>((*this)[3] ^ mask[3])
        };
    }

    
    /**
     * Bitwise NOT operator for `ipv4` objects.
     *
     * @return A new `ipv4` object containing the result of the bitwise NOT operation.
     *
     * @throws None.
     */
    ipv4 operator~ () const
    {
        return {
            static_cast<uint8_t>(~(*this)[0]),
            static_cast<uint8_t>(~(*this)[1]),
            static_cast<uint8_t>(~(*this)[2]),
            static_cast<uint8_t>(~(*this)[3])
        };
    }


    operator std::string () const
    {
        return std::format("{:d}.{:d}.{:d}.{:d}", (*this)[0], (*this)[1], (*this)[2], (*this)[3]);
    }


    operator uint32_t () const
    {
        return { 
            static_cast<uint32_t>((*this)[0]) << 0  |
            static_cast<uint32_t>((*this)[1]) << 8  |
            static_cast<uint32_t>((*this)[2]) << 16 |
            static_cast<uint32_t>((*this)[3]) << 24
        };
    }

};

}
