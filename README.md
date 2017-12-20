miniDHT
=======

## Authors

 - anirul (Frederic DUBOUCHET)
 - mic (Michael JAUSSI)

## Description

My aim was to make a DHT (Distributed Hash Table) written in C++ with not too many dependencies. The DHT itself is based on Kademlia protocol. This is the implemetation used in many project including BitTorrent.

 - miniDHT/* actual DHT (only part needed if you want to use it).
 - Tests/* various examples.

The cmake file will generate the library and compile the protobuffer file on his own.

## Dependencies

 - boost (ASIO)
 - openSSL (SHA256 AES)
 - sqlite3 (database managment)
 - protobuf (google::protocole serialization)

## TODO list

 - Add a routing algorithm (in receive and send)
 - Find a way to speed it up!
