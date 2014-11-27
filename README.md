miniDHT
=======

##Authors

 - anirul (Frederic DUBOUCHET)
 - mic (Michael JAUSSI)

##Description

My aim was to make a DHT (Distributed Hash Table) written in C++ with not too many dependencies. The DHT itself is based on Kademlia protocol. This is the implemetation used in many project including BitTorrent.

Short description of warious part :

 - miniDHT_* actual DHT (only part needed if you want to use it).
 - *.cpp various examples.

##Dependencies

 - boost (ASIO)
 - openSSL (SHA256 AES)
 - sqlite3 (database managment)
 - protobuf (google::protocole serialization)

##TODO list

 - watch clean up of memory at the exit of dialogs
 - Finish the GUI (add download etc...)
 - Add a routing algorithm (in receive and send)
 - Find a way to speed it up!