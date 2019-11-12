spook-your-os
===================

`spook-your-os` is an implementation of the deaddrop lab for CS598CF Fall 2019. It is a covert channel built by exploiting contention in the RNG module of recent Intel processors for fun and profit. 

----------

Build Instructions
-------------

Run `make` to build the targets `trojan` and `spy`. You should run `spy` first, and then trojan. The spy is started by running `taskset -c 0 ./spy` and the trojan is started by running `taskset -c 1 ./trojan`.

Protocol Specification
-------------

1. The spy is started.
2. The trojan is started.
3. The trojan wants to signal to the spy that it is alive.
4. To do so, it starts spamming the RNG using the `rdseed` instruction. It does so for ~ 700000 clock cycles.
5. The spy is meanwhile probing the RNG to figure out if it can successfully get random numbers or not.
6. If the rate of unsuccessful attempts to access randomness from the RNG becomes higher than a threshold, the spy now knows that the trojan is alive.
7. The spy then switches to the listening mode where it follows the following protocol to get the data. 
    1. It detects low contention followed by high contention.
    2. Once it has detected high contention, it starts listening for data.
    3. Data is detected by listening for phase transitions similar to how Manchester decoding works, though the initial detection is more timing based.
8. The protocol is:
    1. Every packet starts with a 1.
    2. There are two types of packets: length/header packet (`fitf_t`) and data packet.
    3. The length/header packet has 1 as its first bit, and then the encoded length value. The maximum length of any given message is 32 bits. The length of this packet is a total of 8 bits, where the first bit is just `1` as mentioned earlier. 
    4. The data packet also has 1 as its first bit, a 0/1 as the second, and the bit representation of 1 byte of information as the rest. The second bit (0/1) is a synchronization bit. Every data packet alternates between 0 and 1. So, if the first packet sent had a 0 for the sync bit, the next packet sent would have a 1 for the sync, the following packet sent would have a 0 for the sync and so on. 
    5. The spy decodes these packets and discards those packets that don't satisfy the protocol. 
    
Example packet(s)
-------------
If the world *rip* is supposed to be sent from the trojan to the spy, there would be a total of 5 packets that would get sent. 

The first packet would be the length packet. That would look like 
`0 0 0 0 0 1 1 1`. The least significant bit (LSB) is the first bit sent. It is always 1 for every packet. Next two bits are 1 because that is the length of the message *rip*. Then the remaining message length is padded out with 0s.

The following packets are data packets. The first data packet encodes the character *r*, and looks like `0 1 1 1 0 0 1 0 0 1`. The LSB is again 1. The bit immediately after the LSB is 0 because it is the sync bit. The rest eight bits are the binary encoding of *r*.

The second data packet is `0 1 1 0 1 0 0 1 1 1`. Again, the LSB is 1, the sync bit is now 1, since it is supposed to alternate between successive packets, and the remaining eight bits are the binary encoding of *i*.

Finally, the third data packet is `0 1 1 1 0 0 0 0 0 1`. LSB is 1, sync bit is now back to 0, and the rest is the binary encoding of *p*.

Challenges
-------------
The biggest challenge was synchronizing the initial bits of a packet and then maintaining the synchronization through the entire packet. It was also a challenge to figure out why there were occasional order of magnitude jumps in rdtsc. I tried several approaches such as setting the frequency scaling to be constant, disabling Turbo Boost, disabling Hyperthreading and so forth, but none of them worked. I finally figured out it was because of something called System Management Mode which creates interrupts below the OS layer in order to efficient manage peripherals like USB devices. I was using a USB keyboard and mouse, and that was triggering frequent interrupts.
