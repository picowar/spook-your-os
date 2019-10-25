# Deaddrop

Run `make` to build the targets `trojan` and `spy`. You should run `spy` first, and then trojan.

The protocol is:
1. The spy is started.
2. The trojan is started.
3. The trojan wants to tell the spy that it is alive.
4. To do so, it starts spamming the RNG using `rdseed` instruction.
5. The spy is meanwhile probing the RNG to figure out if it can successfully get random numbers or not.
6. If the rate of unsuccessful attempts becomes higher than a threshold, the spy now knows that the trojan is alive.
7. The spy then switches to the listening mode where it follows the following protocol to get the data, which is encoding using Manchester encoding.
    1. It detects low contention followed by high contention.
    2. Once it has detected high contention, it starts listening for data.
8. The protocol is:
    1. Every packet starts with a 1.
    2. There are two types of packets: length (`fitf_t`) and data packet.
    3. The length packet has a 1 as its first bit, and then the encoded length value.
    4. The data packet has a 1 as its first bit, a 1/0 as the second, and the bit representation of 1 byte of information as the rest.
    5. The spy decodes these packets and discards those packets that don't satisfy the protocol.
  
 

